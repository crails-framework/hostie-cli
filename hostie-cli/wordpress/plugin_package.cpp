#include "plugin_package.hpp"
#include "plugin_header.hpp"
#include <crails/cli/process.hpp>
#include "temporary_directory.hpp"
#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <vector>

using namespace std;
using namespace Wordpress;
namespace fs = std::filesystem;

namespace
{
  const size_t max_entries = 50000;

  PackageResult failure(PackageResult::Status status, const string& message)
  {
    PackageResult result;

    result.status = status;
    result.message = message;
    return result;
  }

  // Only what unzip created, in a directory nobody else can enter: no link,
  // no device, no socket
  bool is_plain_tree(const fs::path& root, string& problem)
  {
    error_code error;
    size_t count = 0;

    for (fs::recursive_directory_iterator it(root, error), end ; !error && it != end ; it.increment(error))
    {
      const fs::file_status status = fs::symlink_status(it->path(), error);

      if (++count > max_entries)
      {
        problem = "the archive holds too many files";
        return false;
      }
      if (!fs::is_regular_file(status) && !fs::is_directory(status))
      {
        problem = "the archive holds a link or a special file: " + fs::relative(it->path(), root).string();
        return false;
      }
    }
    if (error)
      problem = "could not read the extracted files: " + error.message();
    return !error;
  }

  // The one directory the archive is made of ("__MACOSX" is what macOS adds)
  bool find_plugin_directory(const fs::path& extracted, fs::path& result)
  {
    vector<fs::path> directories;
    error_code error;

    for (const auto& entry : fs::directory_iterator(extracted, error))
    {
      if (entry.path().filename() == "__MACOSX")
        continue ;
      if (!fs::is_directory(entry.path(), error))
        return false;
      directories.push_back(entry.path());
    }
    if (directories.size() != 1)
      return false;
    result = directories.front();
    return true;
  }

  bool apply_ownership(const fs::path& root, const string& owner, const string& group, string& problem)
  {
    uid_t uid = static_cast<uid_t>(-1);
    gid_t gid = static_cast<gid_t>(-1);
    vector<fs::path> paths{root};
    error_code error;

    if (!owner.empty())
    {
      const passwd* user = getpwnam(owner.c_str());

      if (!user)
      {
        problem = "unknown user " + owner;
        return false;
      }
      uid = user->pw_uid;
    }
    if (!group.empty())
    {
      const struct group* entry = getgrnam(group.c_str());

      if (!entry)
      {
        problem = "unknown group " + group;
        return false;
      }
      gid = entry->gr_gid;
    }
    for (const auto& entry : fs::recursive_directory_iterator(root, error))
      paths.push_back(entry.path());
    for (const fs::path& path : paths)
    {
      const bool directory = fs::is_directory(fs::symlink_status(path, error));

      fs::permissions(
        path,
        directory
          ? fs::perms::owner_all | fs::perms::group_read | fs::perms::group_exec
          : fs::perms::owner_read | fs::perms::owner_write | fs::perms::group_read,
        fs::perm_options::replace, error
      );
      if (error)
      {
        problem = "could not change the permissions of " + path.string() + ": " + error.message();
        return false;
      }
      if ((uid != static_cast<uid_t>(-1) || gid != static_cast<gid_t>(-1)) && lchown(path.c_str(), uid, gid) != 0)
      {
        problem = "could not change the owner of " + path.string();
        return false;
      }
    }
    return true;
  }

  PackageResult install_from_zip(const fs::path& zip, const fs::path& target, const string& owner, const string& group, TemporaryDirectory& work)
  {
    const fs::path extracted = work.path() / "extracted";
    const fs::path previous = work.path() / "previous";
    fs::path plugin_directory;
    PackageResult result;
    string problem;
    error_code error;

    fs::create_directory(extracted, error);
    if (error)
      return failure(PackageResult::InstallFailed, "could not create " + extracted.string());
    if (!Crails::run_command({"unzip", {"-q", "-o", zip.string(), "-d", extracted.string()}}))
      return failure(PackageResult::InvalidPackage, "could not extract the archive");
    if (!is_plain_tree(extracted, problem))
      return failure(PackageResult::InvalidPackage, problem);
    if (!find_plugin_directory(extracted, plugin_directory))
      return failure(PackageResult::InvalidPackage, "the archive should hold exactly one directory");
    if (auto header = find_plugin_header(plugin_directory))
      result.version = header->version;
    else
      return failure(PackageResult::InvalidPackage, "no plugin header found: this is not a WordPress plugin");
    if (!apply_ownership(plugin_directory, owner, group, problem))
      return failure(PackageResult::InstallFailed, problem);

    const bool replacing = fs::symlink_status(target, error).type() != fs::file_type::not_found;

    if (replacing)
    {
      fs::rename(target, previous, error);
      if (error)
        return failure(PackageResult::InstallFailed, "could not move " + target.string() + " aside: " + error.message());
    }
    fs::rename(plugin_directory, target, error);
    if (error)
    {
      const string message = error.message();

      if (replacing)
        fs::rename(previous, target, error);
      return failure(PackageResult::InstallFailed, "could not move the plugin into place: " + message);
    }
    return result;
  }

  bool starts_with(const string& value, const string& prefix)
  {
    return value.compare(0, prefix.size(), prefix) == 0;
  }
}

PackageResult Wordpress::install_plugin_zip(const fs::path& zip, const fs::path& target, const string& owner, const string& group)
{
  TemporaryDirectory work(target.parent_path());

  if (!work)
    return failure(PackageResult::InstallFailed, "could not create a temporary directory in " + target.parent_path().string());
  return install_from_zip(zip, target, owner, group, work);
}

PackageResult Wordpress::download_and_install_plugin(const string& url, bool allow_http, const fs::path& target, const string& owner, const string& group)
{
  TemporaryDirectory work(target.parent_path());
  const fs::path zip = work.path() / "package.zip";
  const string protocols = allow_http ? "=http,https" : "=https";

  if (!work)
    return failure(PackageResult::InstallFailed, "could not create a temporary directory in " + target.parent_path().string());
  if (!starts_with(url, "https://") && !(allow_http && starts_with(url, "http://")))
    return failure(PackageResult::DownloadFailed, "unsupported URL (https is required)");
  if (!Crails::run_command({"curl", {
        "--fail", "--silent", "--show-error", "--location", "--globoff",
        "--max-time", "180", "--proto", protocols, "--proto-redir", protocols,
        "--user-agent", "hostie-cli", "--output", zip.string(), url
      }}))
    return failure(PackageResult::DownloadFailed, "could not download " + url);
  return install_from_zip(zip, target, owner, group, work);
}
