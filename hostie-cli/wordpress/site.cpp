#include "../environment.hpp"
#include "../httpd.hpp"

using namespace std;
using namespace HttpServer;

namespace Wordpress
{
  void site_initializer(const InstanceEnvironment& environment, Site& site)
  {
    bool post_install = environment.get_variable("WP_INSTALLED") == "1";
    bool strict_ssl = environment.get_variable("STRICT_SSL_POLICY") == "1";
    string target = environment.get_variable("VAR_DIRECTORY");

    site.locations.push_back(Location{
      "~* ^/wp-content/uploads/.*\.(exe|html|htm|php|php3|php4|php5|phtml)$",
      target, ForbiddenLocation, (strict_ssl ? SslRequired : NoSslState)
    });
    site.locations.push_back(Location{
      "/", target, PhpFpmLocation, (strict_ssl ? SslRequired : NoSslState)
    });
    site.locations.push_back(Location{
      "~* \\.(js|css|png|jpg|jpeg|gif|ico|avif|webp|webm|ogg|oga|mp3|mp4)$",
      target, DirectoryLocation, (strict_ssl ? SslRequired : NoSslState),
      {
        "expires max;"
        "log_not_found off;"
      }
    });
    if (post_install)
    {
      site.locations.push_back(Location{"/wp-admin/install.php",      target, ForbiddenLocation, (strict_ssl ? SslRequired : NoSslState)});
      site.locations.push_back(Location{"/wp-admin/network/menu.php", target, ForbiddenLocation, (strict_ssl ? SslRequired : NoSslState)});
      site.locations.push_back(Location{"/wp-admin/user/menu.php",    target, ForbiddenLocation, (strict_ssl ? SslRequired : NoSslState)});
      site.locations.push_back(Location{"/wp-admin/admin-bar.php",    target, ForbiddenLocation, (strict_ssl ? SslRequired : NoSslState)});
    }
    if (!strict_ssl)
    {
      site.locations.push_back(Location{
        "/wp-admin", target, PhpFpmLocation, SslOmit | SslRequired
      });
    }
  }
}
