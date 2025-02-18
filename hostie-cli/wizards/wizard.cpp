#include "wizard.hpp"
#include "../file_ownership.hpp"
#include <crails/cli/process.hpp>
#include <crails/cli/filesystem.hpp>
#include <boost/process.hpp>
#include <iostream>

using namespace std;

static string get_uname()
{
  string result;

  Crails::run_command("uname", result);
  cout << "Detected system: " << result;
  return result;
}

static pair<string,string> get_lsb()
{
  pair<string,string> result;

  Crails::run_command("lsb_release -i", result.first);
  Crails::run_command("lsb_release -r", result.second);
  cout << "Detected distribution: " << result.first;
  cout << "Detected release: " << result.second;
  return result;
}

char WizardBase::system_matches(const string_view distribution, const string_view version)
{
  static const string uname = get_uname();

  if (uname.find("Linux") != string::npos)
  {
    if (Crails::require_command("lsb_release"))
    {
      static const pair<string,string> lsb = get_lsb();

      if (string_view(lsb.first).find(distribution) != string_view::npos)
        return string_view(lsb.second).find(version) != string_view::npos ? 2 : 1;
    }
  }
  else if (uname.find("FreeBSD") != string::npos)
    return distribution == "FreeBSD" ? 1 : 0;
  return 0;
}

bool WizardBase::download_file(const string_view url, const filesystem::path& target) const
{
  stringstream download_command;

  download_command << "curl"
    << " -o " << target
    << " -L " << quoted(url);
  boost::process::child download_process(download_command.str());
  download_process.wait();
  return download_process.exit_code() == 0;
}

bool WizardBase::download_tar_archive(const string_view url) const
{
  if (Crails::require_command("curl") && Crails::require_command("tar"))
  {
    char tmp_filename[L_tmpnam];
    filesystem::path tmp_filepath = std::tmpnam(tmp_filename);

    if (download_file(url, tmp_filepath))
    {
      stringstream extract_command;

      extract_command << "tar xf " << tmp_filepath;
      boost::process::child extract_process(extract_command.str());
      extract_process.wait();
      filesystem::remove(tmp_filepath);
      if (extract_process.exit_code() == 0)
        return true;
      else
        cerr << "failed to extract " << url << endl;
    }
    else
      cerr << "failed to download " << url << endl;
  }
  return false;
}

bool WizardBase::download_zip_archive(const string_view url) const
{
  if (Crails::require_command("curl") && Crails::require_command("zip"))
  {
    char tmp_filename[L_tmpnam];
    filesystem::path tmp_filepath = std::tmpnam(tmp_filename);

    if (download_file(url, tmp_filepath))
    {
      stringstream extract_command;

      extract_command << "unzip " << tmp_filepath;
      boost::process::child extract_process(extract_command.str());
      extract_process.wait();
      filesystem::remove(tmp_filepath);
      if (extract_process.exit_code() == 0)
        return true;
      else
        cerr << "failed to extract " << url << endl;
    }
    else
      cerr << "failed to download " << url << endl;
  }
  return false;
}

bool WizardBase::extract_source(const std::string_view url, const std::string_view target_name, const std::filesystem::path& target) const
{
  if (download_tar_archive(url))
  {
    if (!filesystem::exists(target))
      filesystem::create_directories(target.parent_path());
    else
      filesystem::remove_all(target);
    if (Crails::move_file(target_name, target))
      return true;
    else
      cerr << "Failed to move " << target_name << " to " << target << endl;
  }
  else
    cerr << "Failed to download " << url << endl;
  return false;
}

bool WizardBase::add_package_requirement(const string_view name, const string_view url)
{
  if (package_urls.find(name) == package_urls.end())
  {
    package_urls.emplace(name, url);
    return true;
  }
  return false;
}

bool WizardBase::install_package_from_urls() const
{
  filesystem::path tmp_path("/tmp/dependency_package");

  for (auto it = package_urls.begin() ; it != package_urls.end() ; ++it)
  {
    const string_view package_name = it->first;
    const string_view package_url = it->second;

    if (package_url.length() == 0)
      continue ;
    if (download_file(package_url, tmp_path))
    {
      stringstream command;

      command << "dpkg -i " << tmp_path;
      if (system(command.str().c_str()) == 0)
        continue ;
    }
    cerr << "failed to install package " << package_name << endl;
    return false;
  }
  return true;
}

bool WizardBase::apply_web_permissions(const filesystem::path& target) const
{
  auto uid = Crails::uid_from_username(HostieVariables::global->variable("web-user"));
  auto gid = Crails::gid_from_groupname(HostieVariables::global->variable("web-group"));
  auto directory_permissions =
    filesystem::perms::owner_all |
    filesystem::perms::group_read | filesystem::perms::group_exec;
  auto exe_permissions = directory_permissions;
  auto file_permissions =
    filesystem::perms::owner_all | filesystem::perms::group_read;

  for (const auto& entry : filesystem::directory_iterator(target))
  {
    const filesystem::path path = entry.path();

    Crails::chown(path, uid);
    Crails::chgrp(path, gid);
    if (filesystem::is_directory(path))
    {
      filesystem::permissions(path, directory_permissions);
      apply_web_permissions(path);
    }
    else if (path.extension() == ".php")
    {
      filesystem::permissions(path, exe_permissions);
    }
    else
    {
      filesystem::permissions(path, file_permissions);
    }
  }
  return true;
}

bool WizardBase::checksum(const string_view command, const filesystem::path& filepath, const string_view hash)
{
  Crails::ExecutableCommand sum_command(
    {string(command), vector<string>{filepath.string()}}
  );
  string result;

  if (Crails::run_command(sum_command, result))
  {
    const string_view verify_sum = string_view(
      result.c_str(), result.find(' ')
    );

    if (hash == verify_sum)
      return true;
    else
    {
      cerr << "failed to verify sum for " << filepath << endl
            << "\tExpected: " << hash << endl
            << "\tReceived: " << verify_sum << endl;
    }
  }
  else
    cerr << "failed to read sum for " << filepath << endl;
  return false;
}

bool WizardBase::checksum_sha256(const filesystem::path& filepath, const string_view sha256_sum)
{
  return checksum("sha256sum", filepath, sha256_sum);
}

bool WizardBase::gpg_dearmor(const filesystem::path& source, const filesystem::path& output)
{
  ostringstream command;

  command << "cat " << source << " | gpg --dearmor | tee " << output;
  if (system(command.str().c_str()) == 0)
    return true;
  else
    cerr << "failed to run gpg --dearmor on " << source << endl;
  return false;
}
