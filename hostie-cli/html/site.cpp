#include "../environment.hpp"
#include "../httpd.hpp"

using namespace std;
using namespace HttpServer;

namespace Html 
{
  void site_initializer(const InstanceEnvironment& environment, Site& site)
  {
    bool strict_ssl = environment.get_variable("STRICT_SSL_POLICY") == "1";
    string target = environment.get_variable("VAR_DIRECTORY");

    site.locations.push_back(Location{
      "/", target, DirectoryLocation, (strict_ssl ? SslRequired : NoSslState),
      {
        "expires max;"
        "log_not_found off;"
      }
    });
  }
}
