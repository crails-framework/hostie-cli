#include "remove_command.hpp"
#include "../system.hpp"

using namespace std;

namespace Nginx
{
  int RemoveCommand::run()
  {
    if (!filesystem::exists(site_conf_path()) || filesystem::remove(site_conf_path()))
    {
      reload_service("nginx");
      return 0;
    }
    return -1;
  }

  bool remove_site(const string& name)
  {
    RemoveCommand command;
    const char* argv[] = {
      "", "-n", name.c_str()
    };

    command.initialize(3, argv);
    return command.run() == 0;
  }
}
