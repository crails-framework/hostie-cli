#include "source_command.hpp"
#include "sources.hpp"
#include "../wizards/wordpress/wizard.hpp"
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace Wordpress;

int SourceCommand::run()
{
  if (!options.count("version"))
  {
    cerr << "Missing option version" << endl;
    return 2;
  }

  const string version = options["version"].as<string>();

  if (!is_wordpress_version(version))
  {
    cerr << "`" << version << "` is not a version number (expected something like 6.8.3)" << endl;
    return 2;
  }

  const filesystem::path source = source_root() / version;

  if (is_complete_source(source))
  {
    cout << "WordPress " << version << " is already installed" << endl;
    return 0;
  }
  if (filesystem::exists(source))
  {
    // What an earlier download left behind: no instance can be using it
    cout << "Removing an incomplete installation of WordPress " << version << endl;
    filesystem::remove_all(source);
  }
  // The wizard reads the version it installs from the environment
  setenv("WORDPRESS_VERSION", version.c_str(), 1);
  if (Wizard().download_wordpress() && is_complete_source(source))
  {
    cout << "WordPress " << version << " installed" << endl;
    return 0;
  }
  cerr << "could not install WordPress " << version << endl;
  return 4;
}
