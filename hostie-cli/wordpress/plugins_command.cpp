#include "plugins_command.hpp"
#include "plugin_scan.hpp"
#include "sources.hpp"
#include "wordpress_org_client.hpp"
#include "../list_command.hpp"
#include <iostream>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>

using namespace std;
using namespace Wordpress;

filesystem::path find_wordpress_source(const string& version);

static unsigned int count_instances_running(const string& version)
{
  unsigned int count = 0;

  for (const filesystem::path& path : ListCommand::instance_environments())
  {
    InstanceEnvironment other;

    if (path.extension() != ".env")
      continue ;
    other.set_project_name(path.filename().replace_extension().string());
    other.load();
    if (other.get_variable("APPLICATION_TYPE") == "Wordpress" && other.get_variable("WORDPRESS_VERSION") == version)
      ++count;
  }
  return count;
}

static void append_string(ostream& out, const char* key, const string& value)
{
  out << ',' << std::quoted(key) << ':' << std::quoted(value);
}

int PluginsCommand::run()
{
  const bool check = options.count("check") > 0;
  const string version = environment.get_variable("WORDPRESS_VERSION");
  // The version is what tells where the instance gets its WordPress from:
  // WORDPRESS_SRC is only up to date until the instance is upgraded.
  const filesystem::path source = version.empty()
    ? filesystem::path(environment.get_variable("WORDPRESS_SRC"))
    : find_wordpress_source(version);
  const filesystem::path var_directory = environment.get_variable("VAR_DIRECTORY");
  const vector<PluginEntry> plugins = scan_plugins(
    var_directory / "wp-content" / "plugins",
    source / "wp-content" / "plugins"
  );
  optional<string> latest_core;
  map<string, Org::PluginRelease> latest_plugins;
  ostringstream out;
  bool first = true;

  if (check)
  {
    vector<string> slugs;

    for (const PluginEntry& plugin : plugins)
      slugs.push_back(plugin.slug);
    latest_core = Org::latest_core_version();
    latest_plugins = Org::latest_plugin_releases(slugs);
  }

  out << "{\"instance\":" << std::quoted(environment.get_project_name())
      << ",\"wordpress\":{\"version\":" << std::quoted(version)
      << ",\"source\":" << std::quoted(source.string())
      << ",\"shared_by\":" << count_instances_running(version)
      << ",\"installed\":[";
  for (const string& installed : installed_versions(source_root()))
  {
    out << (first ? "" : ",") << std::quoted(installed);
    first = false;
  }
  out << ']';
  if (latest_core)
    append_string(out, "latest", *latest_core);
  out << '}';
  if (check)
    out << ",\"update_check\":" << (latest_core ? "\"ok\"" : "\"failed\"");
  out << ",\"plugins\":[";
  first = true;
  for (const PluginEntry& plugin : plugins)
  {
    auto latest = latest_plugins.find(plugin.slug);

    out << (first ? "" : ",") << "{\"slug\":" << std::quoted(plugin.slug)
        << ",\"scope\":" << (plugin.scope == PluginEntry::Global ? "\"global\"" : "\"local\"");
    if (!plugin.name.empty())
      append_string(out, "name", plugin.name);
    if (!plugin.version.empty())
      append_string(out, "version", plugin.version);
    out << ",\"linked\":" << (plugin.linked ? 1 : 0)
        << ",\"broken\":" << (plugin.broken ? 1 : 0)
        << ",\"overrides_global\":" << (plugin.overrides_global ? 1 : 0);
    if (latest != latest_plugins.end())
    {
      append_string(out, "latest", latest->second.version);
      if (!latest->second.requires_wp.empty())
        append_string(out, "latest_requires", latest->second.requires_wp);
    }
    out << '}';
    first = false;
  }
  out << "]}";
  cout << out.str() << endl;
  return 0;
}
