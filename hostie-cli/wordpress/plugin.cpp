#include "plugin.hpp"
#include "version.hpp"
#include "../wizards/wizard.hpp"
#include "../hostie_variables.hpp"
#include "../environment.hpp"
#include "../list_command.hpp"
#include <crails/cli/with_path.hpp>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace Wordpress;

filesystem::path find_wordpress_source(const string& version);

filesystem::path PluginCommand::wordpress_source() const
{
  if (options.count("version"))
    return find_wordpress_source(options["version"].as<string>());
  return HostieVariables::global->variable("wordpress-src");
}

filesystem::path PluginCommand::plugin_path() const
{
  return wordpress_source() / "wp-content" / "plugins" / options["name"].as<string>();
}

string PluginCommand::wordpress_version() const
{
  if (options.count("version"))
    return options["version"].as<string>();
  return HostieVariables::global->variable_or(
    "wordpress-version",
    WORDPRESS_DEFAULT_VERSION
  );
}

int PluginCommand::run()
{
  if (options.count("url") && options.count("name"))
  {
    wp_folder = wordpress_source();
    if (filesystem::exists(wp_folder))
    {
      pull_plugin(options["url"].as<string>());
      link_plugin();
      return 0;
    }
    else
      cerr << "Could not find wordpress folder " << wp_folder << endl;
  }
  else
    cerr << "Missing options url and/or name" << endl;
  return -1;
}

bool PluginCommand::pull_plugin(const string& url)
{
  Crails::WithPath tmp_path("/tmp/hostie-wp-plugins");
  WizardBase helper;

  if (helper.download_zip_archive(options["url"].as<string>()))
  {
    filesystem::path plugin_path = this->plugin_path();

    if (filesystem::exists(plugin_path))
      cout << "Updating ";
    else
      cout << "Adding ";
    cout << plugin_path << endl;
    filesystem::remove_all(plugin_path);
    filesystem::rename(options["name"].as<string>(), plugin_path);
    return true;
  }
  else
    cerr << "Failed to download url " << url << endl;
  return false;
}

bool PluginCommand::link_plugin()
{
  filesystem::path source = plugin_path();
  const string target_version = wordpress_version();

  cout << "Linking plugin " << source << " with instances of Wordpress " << target_version << endl;
  for (const auto& environment_path : ListCommand::instance_environments())
  {
    const string project_name = environment_path.filename().replace_extension().string();
    InstanceEnvironment environment;

    environment.set_project_name(project_name);
    environment.load();
    if (environment.get_variable("APPLICATION_TYPE") == "Wordpress" &&
        environment.get_variable("WORDPRESS_VERSION") == target_version)
      link_plugin(environment, source);
  }
  return true;
}

bool PluginCommand::link_plugin(const InstanceEnvironment& instance, const filesystem::path& source) const
{
  const filesystem::path var_directory = instance.get_variable("VAR_DIRECTORY");
  const filesystem::path plugins_directory = var_directory / "wp-content" / "plugins";
  const filesystem::path target = plugins_directory / options["name"].as<string>();

  if (filesystem::exists(target))
  {
    if (!filesystem::is_symlink(target))
    {
      cout << "- " << instance.get_project_name() << ": doing nothing; plugin has been manually overridden." << endl;
      return false;
    }
    else
      cout << "- " << instance.get_project_name() << ": updating..." << endl;
    filesystem::remove(target);
  }
  else
    cout << "- " << instance.get_project_name() << ": adding..." << endl;
  filesystem::create_directory_symlink(source, target);
  return true;
}
