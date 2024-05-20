#include "wizard.hpp"
#include <crails/cli/process.hpp>
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
