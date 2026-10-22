#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace Wordpress
{
  // What the comment at the top of a plugin's main file says, read the way
  // WordPress reads it (see get_file_data()).
  struct PluginHeader
  {
    std::string name, version, requires_wp, requires_php;
  };

  // Nothing if there is no "Plugin Name:" line: that is what tells a plugin's
  // main file from any other PHP file.
  std::optional<PluginHeader> parse_plugin_header(std::string_view contents);

  // Same, for a file. Only its first 8 KiB are read, as WordPress does.
  std::optional<PluginHeader> read_plugin_header(const std::filesystem::path& file);

  // A plugin is either a directory (the main file is a .php file at the top
  // of it: <slug>.php is tried first, then the others in alphabetical order),
  // or a single .php file.
  std::optional<PluginHeader> find_plugin_header(const std::filesystem::path& plugin);
}
