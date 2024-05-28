#include "ubuntu.hpp"
#include "../mysql/ubuntu.hpp"
#include "../../php.hpp"
#include <iostream>

using namespace std;
using namespace NextCloud::Ubuntu;

int Wizard::run()
{
  requirements = {
    "php",
    "php-fpm",
    "php-mysql",
    "php-bcmath",     // for passwordless login
    "php-curl",
    "php-ctype",
    "php-dom",
    "php-exif",       // image rotation
    "php-fileinfo",
    "php-gd",
    "php-gmp",
    "php-imagick",
    "php-imap",       // for external user authentication
    "php-intl",       // increases language translation perf
    "php-json",
    "php-mbstring",
    "php-net-ftp",    // for FTP storage / external user authentication
    "php-posix",
    "php-xml",
    "php-zip"
  };
  if (require_wizard<UbuntuMysqlWizard>())
  {
    if (install_requirements())
    {
      if (download_nextcloud())
      {
        string service_name = "php" + php_version() + "-fpm";

        HostieVariables::global->variable("php-fpm-service-name", service_name);
        HostieVariables::global->save();
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
