#include "upgrade.hpp"
#include "folder_install.hpp"
#include "../user.hpp"
#include "../hostie_variables.hpp"
#include "../phpfpm_creator.hpp"
#include "plugin_copy.hpp"
#include "sources.hpp"
#include <iostream>

using namespace std;
using namespace Wordpress;

filesystem::path find_wordpress_source(const string& version);

void MigrateVersionCommand::options_description(boost::program_options::options_description& options) const
{
  LiveInstanceCommand::options_description(options);
  options.add_options()
    ("version,v", boost::program_options::value<string>(), "target version number (eg: 6.2.2)")
    ("skip-global-plugins", "do not carry the global plugins over to the target version");
}

// A version of WordPress has global plugins of its own (they are installed in
// its source). Without them in the target version, the instance would lose its
// global plugins in the move.
static bool carry_global_plugins(const filesystem::path& current_source, const filesystem::path& target_source)
{
  vector<string> errors;
  const vector<string> copied = Wordpress::copy_missing_plugins(
    current_source / "wp-content" / "plugins",
    target_source / "wp-content" / "plugins",
    errors
  );

  for (const string& plugin : copied)
    cout << "Global plugin " << plugin << " copied to " << target_source << endl;
  for (const string& error : errors)
    cerr << error << endl;
  return errors.empty();
}

int MigrateVersionCommand::run()
{
  if (!options.count("version"))
  {
    cerr << "missing target version option" << endl;
    return -1;
  }

  const string current_version = environment.get_variable("WORDPRESS_VERSION");
  const string target_version = options["version"].as<string>();
  // The version is what tells where an instance gets its WordPress from.
  // WORDPRESS_SRC is only what it was created with (it is kept up to date
  // below, but instances that were upgraded before that still have the old one).
  const filesystem::path current_source = current_version.empty()
    ? filesystem::path(environment.get_variable("WORDPRESS_SRC"))
    : find_wordpress_source(current_version);
  const filesystem::path target_source = find_wordpress_source(target_version);
  InstanceUser user;

  if (!is_wordpress_version(target_version))
  {
    cerr << "`" << target_version << "` is not a version number (expected something like 6.8.3)" << endl;
    return -1;
  }
  if (current_version == target_version)
  {
    cout << "already running WordPress " << target_version << endl;
    return 0;
  }
  if (!filesystem::exists(current_source))
  {
    cerr << "could not find current wordpress source directory " << current_source << endl;
    return -2;
  }
  if (!is_complete_source(target_source))
  {
    cerr << "could not find target wordpress source directory " << target_source << endl
         << "install using: hostie-cli wordpress source -v " << target_version << endl;
    return -2;
  }
  var_directory = environment.get_variable("VAR_DIRECTORY");
  user.name = environment.get_variable("APPLICATION_USER");
  user.group = HostieVariables::global->variable("web-group");
  // Nothing of the instance has been touched yet: a failure here changes nothing
  if (!options.count("skip-global-plugins") && !carry_global_plugins(current_source, target_source))
    return 1;
  try
  {
    if (unprepare_wordpress(current_source, user) && prepare_wordpress(target_source, user))
    {
      environment.set_variable("WORDPRESS_VERSION", target_version);
      environment.set_variable("WORDPRESS_SRC", target_source.string());
      environment.save();
      PhpFpmCreator::restart_php_fpm();
      return 0;
    }
  }
  catch (const filesystem::filesystem_error& error)
  {
    cerr << error.what() << endl;
  }
  // The instance has lost its WordPress, or half of it: put the old one back
  cerr << "could not move the instance to WordPress " << target_version << ", putting " << current_version << " back" << endl;
  try
  {
    unprepare_wordpress(target_source, user);
    if (prepare_wordpress(current_source, user))
      cerr << "the instance runs WordPress " << current_version << " again" << endl;
    else
      cerr << "could not put WordPress " << current_version << " back: the instance needs attention" << endl;
  }
  catch (const filesystem::filesystem_error& error)
  {
    cerr << error.what() << endl << "could not put WordPress " << current_version << " back: the instance needs attention" << endl;
  }
  return 1;
}

bool MigrateVersionCommand::unprepare_wordpress(const filesystem::path& wordpress_source, const InstanceUser& user)
{
  return FolderInstaller{
    user,
    var_directory,
    wordpress_source
  }.uninstall();
}

bool MigrateVersionCommand::prepare_wordpress(const filesystem::path& wordpress_source, const InstanceUser& user)
{
  return FolderInstaller{
    user,
    var_directory,
    wordpress_source
  }.install();
}
