#include "plugin.hpp"
#include "plugin_package.hpp"
#include "wordpress_org.hpp"
#include "version.hpp"
#include "../hostie_variables.hpp"
#include "../environment.hpp"
#include "../list_command.hpp"
#include <algorithm>
#include <iostream>

using namespace std;
using namespace Wordpress;

filesystem::path find_wordpress_source(const string& version);

filesystem::path PluginCommand::wordpress_source() const
{
  if (options.count("version"))
    return find_wordpress_source(options["version"].as<string>());
  return HostieVariables::global->variable("wordpress-source");
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

string PluginCommand::download_url() const
{
  if (options.count("url"))
    return options["url"].as<string>();
  return Org::plugin_download_url(
    options["name"].as<string>(),
    options.count("release") ? options["release"].as<string>() : string()
  );
}

int PluginCommand::run()
{
  if (!options.count("name"))
  {
    cerr << "Missing option name" << endl;
    return BadOptions;
  }

  const string slug = options["name"].as<string>();

  // The slug ends up in a path that gets replaced: nothing but a slug will do
  if (!Org::is_plugin_slug(slug))
  {
    cerr << "Invalid plugin name `" << slug << "` (expected a slug, like `akismet`)" << endl;
    return BadOptions;
  }
  if (options.count("url") && options.count("release"))
  {
    cerr << "Options url and release cannot be used together" << endl;
    return BadOptions;
  }
  if (options.count("release") && !Org::is_plugin_release(options["release"].as<string>()))
  {
    cerr << "Invalid release `" << options["release"].as<string>() << '`' << endl;
    return BadOptions;
  }
  if (options.count("instance"))
  {
    if (options.count("version"))
    {
      cerr << "Options instance and version cannot be used together: the instance has its own version" << endl;
      return BadOptions;
    }
    return install_local(slug, download_url());
  }
  return install_global(slug, download_url());
}

int PluginCommand::report(int status, const string& slug, const string& message, const string& version)
{
  if (status == Success)
    cout << "Installed " << slug << ' ' << version << endl;
  else
    cerr << "Could not install " << slug << ": " << message << endl;
  return status;
}

static int exit_code_for(const PackageResult& result)
{
  switch (result.status)
  {
  case PackageResult::Installed:      return PluginCommand::Success;
  case PackageResult::DownloadFailed: return PluginCommand::DownloadFailed;
  case PackageResult::InvalidPackage: return PluginCommand::InvalidPackage;
  case PackageResult::InstallFailed:  break ;
  }
  return PluginCommand::InstallFailed;
}

// An explicit url may be plain http (it was chosen by whoever runs the
// command). The default one never is.
static bool is_explicit_url(const boost::program_options::variables_map& options)
{
  return options.count("url") > 0;
}

int PluginCommand::install_global(const string& slug, const string& url)
{
  const filesystem::path source = wordpress_source();
  const filesystem::path target = source / "wp-content" / "plugins" / slug;
  PackageResult result;

  if (!filesystem::is_directory(source / "wp-content" / "plugins"))
  {
    cerr << "Could not find wordpress folder " << source << endl;
    return NotFound;
  }
  result = download_and_install_plugin(
    url, is_explicit_url(options), target,
    HostieVariables::global->variable_or("web-user", ""),
    HostieVariables::global->variable_or("web-group", "")
  );
  if (result.status != PackageResult::Installed)
    return report(exit_code_for(result), slug, result.message, "");
  report(Success, slug, "", result.version);
  link_plugin(source / "wp-content" / "plugins" / slug, slug);
  return Success;
}

int PluginCommand::install_local(const string& slug, const string& url)
{
  InstanceEnvironment environment;
  PackageResult result;

  environment.set_project_name(options["instance"].as<string>());
  if (!environment.already_exists())
  {
    cerr << "cannot find file " << environment.get_path() << endl;
    return NotFound;
  }
  environment.load();
  if (environment.get_variable("APPLICATION_TYPE") != "Wordpress")
  {
    cerr << options["instance"].as<string>() << " is not a Wordpress instance" << endl;
    return NotFound;
  }

  const filesystem::path plugins_directory = filesystem::path(environment.get_variable("VAR_DIRECTORY")) / "wp-content" / "plugins";
  const filesystem::path target = plugins_directory / slug;

  if (!filesystem::is_directory(plugins_directory))
  {
    cerr << "Could not find " << plugins_directory << endl;
    return NotFound;
  }
  // The instance gets this plugin from the source of its WordPress version:
  // a local copy would have to replace that link, which is not what this
  // command is for.
  if (filesystem::is_symlink(target))
  {
    cerr << slug << " is a global plugin for " << options["instance"].as<string>() << ": update it globally, or remove the link first" << endl;
    return GlobalPluginLinked;
  }
  result = download_and_install_plugin(
    url, is_explicit_url(options), target,
    environment.get_variable("APPLICATION_USER"),
    HostieVariables::global->variable_or("web-group", "")
  );
  return report(exit_code_for(result), slug, result.message, result.version);
}

bool PluginCommand::link_plugin(const filesystem::path& source, const string& slug)
{
  const string target_version = wordpress_version();

  cout << "Linking plugin " << source << " with instances of Wordpress " << target_version << endl;
  for (const auto& environment_path : ListCommand::instance_environments())
  {
    const string project_name = environment_path.filename().replace_extension().string();
    InstanceEnvironment environment;

    if (environment_path.extension() != ".env")
      continue ;
    environment.set_project_name(project_name);
    environment.load();
    if (environment.get_variable("APPLICATION_TYPE") == "Wordpress" &&
        environment.get_variable("WORDPRESS_VERSION") == target_version)
      link_plugin(environment, source, slug);
  }
  return true;
}

bool PluginCommand::link_plugin(const InstanceEnvironment& instance, const filesystem::path& source, const string& slug) const
{
  const filesystem::path var_directory = instance.get_variable("VAR_DIRECTORY");
  const filesystem::path plugins_directory = var_directory / "wp-content" / "plugins";
  const filesystem::path target = plugins_directory / slug;
  error_code error;

  if (filesystem::symlink_status(target, error).type() != filesystem::file_type::not_found)
  {
    if (!filesystem::is_symlink(target))
    {
      cout << "- " << instance.get_project_name() << ": doing nothing; plugin has been manually overridden." << endl;
      return false;
    }
    cout << "- " << instance.get_project_name() << ": updating..." << endl;
    filesystem::remove(target, error);
  }
  else
    cout << "- " << instance.get_project_name() << ": adding..." << endl;
  filesystem::create_directory_symlink(source, target);
  if (error)
    cerr << "- " << instance.get_project_name() << ": could not link the plugin: " << error.message() << endl;
  return !error;
}
