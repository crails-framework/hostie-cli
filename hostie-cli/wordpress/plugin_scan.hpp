#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace Wordpress
{
  // One plugin, as an instance sees it.
  //
  // A global plugin lives in the source of the version of WordPress the
  // instance runs (<wordpress-src>/<version>/wp-content/plugins/<slug>) and is
  // linked into the instance's wp-content/plugins: one copy for all the
  // instances of that version. A local plugin is a copy that sits in the
  // instance's own wp-content/plugins.
  struct PluginEntry
  {
    enum Scope { Global, Local };

    std::string slug;            // the directory name, or the file name without ".php"
    Scope       scope = Local;
    std::string name, version;   // from the header: empty if none could be read
    // Global only: the instance has no link to it (it was installed after the
    // instance linked its plugins, and linking it failed or was skipped)
    bool linked = true;
    // Global only: the link leads nowhere
    bool broken = false;
    // Local only: a global plugin has the same slug, and the local copy wins
    bool overrides_global = false;
  };

  // `instance_plugins`: <var directory>/wp-content/plugins
  // `source_plugins`:   <wordpress-src>/<version>/wp-content/plugins
  // Sorted by slug. Hidden entries and index.php (the "silence is golden"
  // file) are not plugins.
  std::vector<PluginEntry> scan_plugins(const std::filesystem::path& instance_plugins, const std::filesystem::path& source_plugins);

  // True if `path` is `root`, or something inside of it (a comparison of path
  // components, not of strings: "/a/bc" is not inside "/a/b")
  bool is_inside(const std::filesystem::path& path, const std::filesystem::path& root);
}
