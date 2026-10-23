#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace Wordpress
{
  // The global plugins live with the version of WordPress they were installed
  // for (see plugin_scan.hpp). An instance that moves to another version needs
  // them there too, or it loses them.
  //
  // Copies every plugin of `from_plugins` that `to_plugins` does not have yet
  // (what `to_plugins` has already, bundled with that WordPress or installed
  // since, is left alone). Owner, group and modes are kept (cp -a), and each
  // plugin is built next to its destination and renamed into place: a plugin
  // is either there whole, or not at all.
  //
  // Returns the slugs that were copied. `errors` gets one message per plugin that
  // could not be, and the others are still copied.
  std::vector<std::string> copy_missing_plugins(
    const std::filesystem::path& from_plugins,
    const std::filesystem::path& to_plugins,
    std::vector<std::string>& errors
  );
}
