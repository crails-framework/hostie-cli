#include "../environment.hpp"
#include "../httpd.hpp"
#include <crails/utils/semantics.hpp>

using namespace std;
using namespace HttpServer;

namespace Odoo
{
  void site_initializer(const InstanceEnvironment& environment, Site& site)
  {
    string target_prefix = Crails::dasherize(environment.get_project_name());
    string target_web = target_prefix + "-web";
    string target_chat = target_prefix + "-chat";
    unsigned short port = 8069;
    unsigned short chat_port = 8072;

    site.custom_settings = {
      "client_max_body_size 0;",
      "proxy_connect_timeout 720s;",
      "proxy_send_timeout 720s;"
    };

    site.upstreams.push_back(Upstream{
      target_web, "127.0.0.1", port
    });
    site.upstreams.push_back(Upstream{
      target_chat, "127.0.0.1", chat_port
    });

    site.locations.push_back(Location{
      "/", target_web, AppProxyLocation, SslRequired
    });
    site.locations.push_back(Location{
      "/longpolling", target_chat, AppProxyLocation, SslRequired
    });
    site.locations.push_back(Location{
      "~* /web/static", target_web, AppProxyLocation, SslRequired,
      {
        "proxy_cache_valid 200 90m",
        "proxy_buffering on",
        "expires 864000"
      }
    });
  }
}
