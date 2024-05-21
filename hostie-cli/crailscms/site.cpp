#include "../environment.hpp"
#include "../httpd.hpp"
#include <crails/utils/semantics.hpp>

using namespace std;
using namespace HttpServer;

namespace CrailsCMS
{
  void site_initializer(const InstanceEnvironment& environment, Site& site)
  {
    bool strict_ssl = environment.get_variable("STRICT_SSL_POLICY") == "1";
    string target = Crails::dasherize(environment.get_project_name()) + "-web";
    unsigned short port = stoi(environment.get_variable("APPLICATION_PORT"));

    site.upstreams.push_back(Upstream{
      target, "127.0.0.1", port
    });

    site.locations.push_back(Location{
      "/", target, AppProxyLocation, (strict_ssl ? SslRequired : NoSslState)
    });
    if (!strict_ssl)
    {
      site.locations.push_back(Location{
        "/userspace", target, AppProxyLocation, SslOmit | SslRequired
      });
      site.locations.push_back(Location{
        "/admin", target, AppProxyLocation, SslOmit | SslRequired
      });
    }
  }
}
