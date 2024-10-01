#include "upgrade.hpp"
#include "folder_install.hpp"
#include "../user.hpp"
#include "../hostie_variables.hpp"
#include "../phpfpm_creator.hpp"

using namespace std;
using namespace Wordpress;

filesystem::path find_wordpress_source(const string& version);

void MigrateVersionCommand::options_description(boost::program_options::options_description& options) const
{
  LiveInstanceCommand::options_description(options);
  options.add_options()
    ("version,v", boost::program_options::value<string>(), "target version number (eg: 6.2.2)");
}

int MigrateVersionCommand::run()
{
  if (options.count("version"))
  {
    const string current_version = environment.get_variable("WORDPRESS_VERSION");
    const string target_version = options["version"].as<string>();
    const filesystem::path current_source = environment.get_variable("WORDPRESS_SRC");
    const filesystem::path target_source = find_wordpress_source(target_version);
    InstanceUser user;

    if (!filesystem::exists(current_source))
    {
      cerr << "could not find current wordpress source directory " << current_source << endl;
      return -2;
    }
    if (!filesystem::exists(target_source))
    {
      cerr << "could not find target wordpress source directory " << target_source << endl
           << "install using: WORDPRESS_VERSION=" << target_version << " hostie-cli wizard wordpress" << endl;
      return -2;
    }
    var_directory = environment.get_variable("VAR_DIRECTORY");
    user.name = environment.get_variable("APPLICATION_USER");
    user.group = HostieVariables::global->variable("web-group");
    if (unprepare_wordpress(current_source, user) &&
        prepare_wordpress(target_source, user))
    {
      environment.set_variable("WORDPRESS_VERSION", target_version);
      environment.save();
      PhpFpmCreator::restart_php_fpm();
      return 0;
    }
    return 1;
  }
  else
    cerr << "missing target version option" << endl;
  return -1;
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
