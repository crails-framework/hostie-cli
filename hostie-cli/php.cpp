#include "php.hpp"
#include "environment.hpp"
#include <crails/cli/process.hpp>

using namespace std;

string php_version()
{
  string php_version;
  string getter = "php --version | grep -e '^PHP' | cut -d' ' -f2 | cut -d'-' -f1 | cut -d'.' -f1,2";

  Crails::run_command({"sh", {{"-c", getter}}}, php_version);
  return php_version.substr(0, php_version.length() - 1);
}

filesystem::path fpm_pool_path(const InstanceEnvironment& environment)
{
  vector<filesystem::path> candidates{
    filesystem::path("/etc/php") / php_version() / "fpm" / "pool.d",
    filesystem::path("/etc/php-fpm.d")
  };
  string filename = environment.get_project_name() + ".conf";

  for (const filesystem::path& candidate : candidates)
  {
    if (filesystem::is_directory(candidate))
      return candidate / filename;
  }
  return filesystem::path();
}
