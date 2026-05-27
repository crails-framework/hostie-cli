#include "freebsd.hpp"
#include "../mysql/freebsd.hpp"
#include "../../php.hpp"
#include <iostream>
#define PHP_PREFIX FREEBSD_PHP_PREFIX

using namespace std;
using namespace Wordpress::FreeBSD;

int Wizard::run()
{
  requirements = {
    PHP_PREFIX,
    PHP_PREFIX "-mysqli",
    PHP_PREFIX "-curl",
    PHP_PREFIX "-exif",
    PHP_PREFIX "-fileinfo",
    PHP_PREFIX "-pecl-imagick",
    PHP_PREFIX "-mbstring",
    PHP_PREFIX "-xml",
    PHP_PREFIX "-zip",

    PHP_PREFIX "-filter",
    PHP_PREFIX "-ftp",
    PHP_PREFIX "-gd",
    PHP_PREFIX "-tokenizer",
    PHP_PREFIX "-zlib"
  };
  if (require_wizard<FreeBSDMysqlWizard>())
  {
    if (install_requirements())
    {
      if (download_wordpress())
      {
        Crails::run_command("service php_fpm onestart") && Crails::run_command("sysrc php_fpm_enable=\"YES\"");
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
