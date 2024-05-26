#include "httpd.hpp"

using namespace std;
using namespace HttpServer;

namespace Wordpress
{
  void site_initializer(const InstanceEnvironment&, Site&);
}

namespace CrailsCMS
{
  void site_initializer(const InstanceEnvironment&, Site&);
}

namespace Odoo
{
  void site_initializer(const InstanceEnvironment&, Site&);
}

namespace NextCloud
{
  void site_initializer(const InstanceEnvironment&, Site&);
}

const map<string, SiteInitializer> Site::initializers{
  {"Wordpress", &Wordpress::site_initializer},
  {"CrailsCMS", &CrailsCMS::site_initializer},
  {"Odoo",      &Odoo::site_initializer},
  {"NextCloud", &NextCloud::site_initializer}
};
