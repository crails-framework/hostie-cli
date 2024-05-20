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

static filesystem::path find_wordpress_source()
{
  if (std::getenv("WORDPRESS_DIRECTORY") != 0)
    return filesystem::weakly_canonical(getenv("WORDPRESS_DIRECTORY"));
  return filesystem::path("/opt/hostie/wordpress-src");
}

static const string wp_config_src = 
  "<?php require_once($_SERVER[\"App-Root\"] . \"/wp-config.php\"); ?>";

bool Wizard::download_wordpress()
{
  if (download_tar_archive("https://wordpress.org/latest.tar.gz"))
  {
    filesystem::path target = find_wordpress_source();

    if (!filesystem::exists(target))
      filesystem::create_directories(target.parent_path());
    else
      filesystem::remove_all(target);
    if (Crails::move_file("wordpress", target))
    {
      if (Crails::write_file("wordpress-installer", (target / "wp-config.php").string(), wp_config_src))
      {
        store.variable("wordpress_src", target.string());
        store.save();
        return true;
      }
    }
    else
      cerr << "Failed to move wordpress to " << target << endl;
  }
  return false;
}
