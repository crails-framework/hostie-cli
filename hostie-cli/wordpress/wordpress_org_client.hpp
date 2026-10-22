#pragma once
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "wordpress_org.hpp"

// The part of wordpress.org that needs the network: curl does the talking
// (hostie-cli already depends on it to download WordPress itself).
namespace Wordpress
{
  namespace Org
  {
    // Nothing if the version could not be found out (no network, an answer
    // that is not what was expected...)
    std::optional<std::string> latest_core_version();

    // The latest release of each plugin, by slug. Plugins that wordpress.org
    // does not know (premium plugins, custom ones) are simply not in the
    // result, and so are all of them if wordpress.org cannot be reached.
    // The requests are made in parallel.
    std::map<std::string, PluginRelease> latest_plugin_releases(const std::vector<std::string>& slugs);

    // What normalize_search() prints, or nothing if the search failed
    std::optional<std::string> search_plugins(const std::string& query, unsigned int page, unsigned int per_page);
  }
}
