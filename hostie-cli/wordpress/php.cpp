#include "php.hpp"
#include "../environment.hpp"
#include <crails/cli/process.hpp>

using namespace std;

namespace Wordpress
{
  string php_version()
  {
    string php_version;
    string getter = "php --version | grep -e '^PHP' | cut -d' ' -f2 | cut -d'-' -f1 | cut -d'.' -f1,2";
    ostringstream command;

    command << "sh -c " << quoted(getter);
    Crails::run_command(command.str(), php_version);
    return php_version.substr(0, php_version.length() - 1);
  }

  filesystem::path fpm_pool_path(const InstanceEnvironment& environment)
  {
    return filesystem::path("/etc/php") / php_version() / (environment.get_project_name() + ".conf");
  }
}
