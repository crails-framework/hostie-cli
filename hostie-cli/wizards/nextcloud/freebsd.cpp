#include "freebsd.hpp"
#include "../mysql/freebsd.hpp"
#include "../../php.hpp"
#define PHP_PREFIX FREEBSD_PHP_PREFIX

using namespace std;
using namespace NextCloud::FreeBSD;

int Wizard::run()
{
  requirements = {
    PHP_PREFIX,
    PHP_PREFIX "-mysqli",
    PHP_PREFIX "-bcmath",       // for passwordless login
    PHP_PREFIX "-curl",
    PHP_PREFIX "-ctype",
    PHP_PREFIX "-dom",
    PHP_PREFIX "-exif",         // image rotation
    PHP_PREFIX "-fileinfo",
    PHP_PREFIX "-gd",
    PHP_PREFIX "-gmp",
    PHP_PREFIX "-pecl-imagick",
    //PHP_PREFIX "-imap",       // for external user authentication
    PHP_PREFIX "-intl",         // increases language translation perf
    //PHP_PREFIX "-json",
    PHP_PREFIX "-mbstring",
    PHP_PREFIX "-ftp",          // for FTP storage / external user authentication
    PHP_PREFIX "-posix",
    PHP_PREFIX "-xml",
    PHP_PREFIX "-zip"
  };
  if (require_wizard<FreeBSDMysqlWizard>())
  {
    if (install_requirements())
    {
      if (download_nextcloud())
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
