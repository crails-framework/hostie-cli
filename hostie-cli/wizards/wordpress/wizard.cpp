#include "wizard.hpp"
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <filesystem>
#include <boost/process.hpp>
#include <crails/cli/process.hpp>
#include <crails/cli/filesystem.hpp>

using namespace std;
using namespace Wordpress;

static const string_view wordpress_url = 
  "https://wordpress.org/latest.tar.gz";
static const string wp_config_src = 
  "<?php require_once($_SERVER[\"App-Root\"] . \"/wp-config.php\"); ?>";

static filesystem::path find_wordpress_source()
{
  if (std::getenv("WORDPRESS_DIRECTORY") != 0)
    return filesystem::weakly_canonical(getenv("WORDPRESS_DIRECTORY"));
  return filesystem::path("/opt/hostie/wordpress-src");
}

bool Wizard::download_wordpress()
{
  const filesystem::path target = find_wordpress_source();

  if (extract_source(wordpress_url, "wordpress", target))
  {
    if (Crails::write_file("wordpress-installer", (target / "wp-config.php").string(), wp_config_src))
    {
      HostieVariables::global->variable("wordpress-source", target.string());
      HostieVariables::global->save();
      if (apply_web_permissions(target))
        return true;
      else
        cerr << "Failed to apply proper permissions on " << target << endl;
    }
    else
      cerr << "Failed to overwrite wp-config.php" << endl;
  }
  return false;
}
