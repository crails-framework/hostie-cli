#include "ubuntu.hpp"
#include "../mysql/ubuntu.hpp"
#include <iostream>

using namespace std;
using namespace Wordpress::Ubuntu;

int Wizard::run()
{
  requirements = {
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

