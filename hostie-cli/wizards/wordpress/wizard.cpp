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

bool Wizard::apply_phpfpm_permissions(const filesystem::path& target)
{
  return Crails::run_command("chgrp -R " + store.variable("web-group") + " \"" + target.string() + '"')
      && Crails::run_command("chown -R " + store.variable("web-user")  + " \"" + target.string() + '"')
      && Crails::run_command("chmod -R o-rwx \"" + target.string() + '"')
      && std::system(("chmod ug+x \"" + target.string() + "\"/**/*.php").c_str()) == 0
      && std::system(("chmod ug+x \"" + target.string() + "\"/*.php").c_str()) == 0;
}

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
        store.variable("wordpress-source", target.string());
        store.save();
        if (apply_phpfpm_permissions(target))
          return true;
        else
          cerr << "Failed to apply proper permissions on " << target << endl;
      }
      else
        cerr << "Failed to overwrite wp-config.php" << endl;
    }
    else
      cerr << "Failed to move wordpress to " << target << endl;
  }
  return false;
}
