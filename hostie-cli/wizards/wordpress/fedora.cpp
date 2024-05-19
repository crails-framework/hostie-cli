#include "fedora.hpp"
#include "../mysql/fedora.hpp"
#include <iostream>

using namespace std;
using namespace Wordpress::Fedora;

int Wizard::run()
{
  requirements = {
    "php-fpm",
    "php-mysqlnd",
    "php-common",
    "php-mbstring",
    "php-xml",
    "php-pecl-imagick",
    "php-pecl-zip"
  };
  if (require_wizard<FedoraMysqlWizard>())
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
