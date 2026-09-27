#include "capabilities.hpp"
#include "../wizards/mysql/platforms.hpp"
#include "../wizards/nextcloud/platforms.hpp"
#include "../wizards/nginx/platforms.hpp"
#include "../wizards/odoo/platforms.hpp"
#include "../wizards/postgres/platforms.hpp"
#include "../wizards/wordpress/platforms.hpp"

using namespace std;

static vector<string> collect_caps()
{
  vector<string> caps;
  map<string, PlatformInstaller> installers{
    {"wordpress", wordpress_platform_installer()},
    {"postgres",  postgres_platform_installer()},
    {"odoo",      odoo_platform_installer()},    
    {"nginx",     nginx_platform_installer()},
    {"nextcloud", nextcloud_platform_installer()},
    {"mysql",     mysql_platform_installer()}
  };

  for (auto it = installers.begin() ; it != installers.end() ; ++it)
  {
    if (it->second.is_installed())
      caps.push_back(it->first);
  }
  return caps;
}

int CapabilitiesCommand::run()
{
  cout << '[';
  for (const string& cap : collect_caps())
    cout << quoted(cap);
  cout << ']';
  return 0;
}
