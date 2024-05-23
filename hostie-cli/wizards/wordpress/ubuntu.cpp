#include "ubuntu.hpp"
#include "../mysql/ubuntu.hpp"
#include "../../wordpress/php.hpp"
#include <iostream>

using namespace std;
using namespace Wordpress::Ubuntu;

int Wizard::run()
{
  requirements = {
    "php",
    "php-fpm",
    "php-mysql",
    "php-curl",
    "php-exif",
    "php-fileinfo",
    "php-imagick",
    "php-json",
    "php-mbstring",
    "php-xml",
    "php-zip"
  };
  if (require_wizard<UbuntuMysqlWizard>())
  {
    if (install_requirements())
    {
      if (download_wordpress())
      {
        string service_name = "php" + Wordpress::php_version() + "-fpm";
        Crails::run_command("systemctl enable " + service_name) && Crails::run_command("systemctl start " + service_name);
        return 0;
      }
    }
    else
      cerr << "failed to install required packages" << endl;
  }
  else
    cerr << "failed to install mysql" << endl;
  return -1;
}

