#pragma once
#include <filesystem>
#include <string>

namespace Wordpress
{
  struct PackageResult
  {
    enum Status
    {
      Installed,
      DownloadFailed, // the archive could not be downloaded
      InvalidPackage, // it is not a plugin, or it holds something that is not safe to install
      InstallFailed   // it is fine, but could not be put in place
    };

    Status      status = Installed;
    std::string message;  // what went wrong
    std::string version;  // what was installed, as its header says
  };

  // Puts the plugin found in the archive `zip` at `target` (a directory that
  // does not exist yet, or is replaced).
  //
  // The archive has to hold one directory, and a plugin header in its main
  // file. It cannot hold anything but directories and regular files: links
  // and special files are refused (they would be followed by what changes
  // the owner of the files, which runs as root).
  //
  // The new version is built next to `target` and moved into place with two
  // renames, so that the plugin is missing for as short a time as possible:
  // WordPress turns off the plugins whose files it cannot find.
  //
  // `owner` and `group` are given to every file ("" leaves it alone), with
  // modes 750 (directories) and 640 (files).
  PackageResult install_plugin_zip(
    const std::filesystem::path& zip,
    const std::filesystem::path& target,
    const std::string& owner,
    const std::string& group
  );

  // Downloads `url` and does what install_plugin_zip does. Only https URLs
  // are accepted, unless `allow_http` is set.
  PackageResult download_and_install_plugin(
    const std::string& url,
    bool allow_http,
    const std::filesystem::path& target,
    const std::string& owner,
    const std::string& group
  );
}
