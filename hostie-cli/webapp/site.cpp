#include "../environment.hpp"
#include "../httpd.hpp"
#include <crails/utils/semantics.hpp>
#include <cstdlib>

using namespace std;
using namespace HttpServer;

namespace WebApp
{
  void site_initializer(const InstanceEnvironment& environment, Site& site)
  {
    bool strict_ssl = environment.get_variable("STRICT_SSL_POLICY") == "1";
    string target = Crails::dasherize(environment.get_project_name()) + "-web";
    unsigned short port = atoi(environment.get_variable("APPLICATION_PORT").c_str());

    site.upstreams.push_back(Upstream{
      target, "127.0.0.1", port
    });

    site.locations.push_back(Location{
      "/", target, AppProxyLocation, (strict_ssl ? SslRequired : NoSslState)
    });
  }
}
