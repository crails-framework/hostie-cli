#pragma once
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// What hostie-cli knows about wordpress.org: where to ask, and how to read
// the answers. Nothing in this file touches the network (see
// wordpress_org_client.hpp for that), so it can be tested on its own.
//
// hostie-cli is the only place that knows the shape of wordpress.org's
// answers: what it prints in turn has a shape of its own, which cms-farm reads.
namespace Wordpress
{
  namespace Org
  {
    // A plugin's slug is what wordpress.org identifies it with
    // ("akismet", "classic-editor"). It becomes a directory name and part of a
    // URL: lowercase letters, digits, hyphens and underscores, starting with a
    // letter or a digit, at most 64 characters.
    bool is_plugin_slug(std::string_view);
    // A plugin's version, as it appears in the name of its archive: letters,
    // digits, dots, hyphens and underscores, starting with a letter or a digit.
    bool is_plugin_release(std::string_view);

    std::string core_version_url();
    std::string plugin_information_url(std::string_view slug);
    std::string plugin_search_url(std::string_view query, unsigned int page, unsigned int per_page);
    // The archive of the latest release when `release` is empty
    std::string plugin_download_url(std::string_view slug, std::string_view release);

    struct PluginRelease
    {
      std::string version;
      std::string requires_wp; // the oldest version of WordPress it runs on, empty if unknown
    };

    // From https://api.wordpress.org/core/version-check/1.7/
    std::optional<std::string> parse_latest_core(std::string_view json);
    // From a plugin_information answer. Nothing for the answers that are an error.
    std::optional<PluginRelease> parse_plugin_release(std::string_view json);
    // From a query_plugins answer, to the JSON `wordpress search` prints:
    //   {"page":1,"pages":4,"total":37,"plugins":[{"slug":"...","name":"...","version":"...",
    //     "short_description":"...","active_installs":1000,"rating":92,"requires":"6.4",
    //     "requires_php":"7.4","tested":"6.8.3","last_updated":"..."}]}
    // `rating` is a percentage. Strings that are empty or unknown are left out.
    // Nothing if the answer is not a list of plugins.
    std::optional<std::string> normalize_search(std::string_view json);
  }
}
