#include "system.hpp"
#include <crails/cli/process.hpp>

using namespace std;

bool start_service(const string& name)
{
#ifdef __FreeBSD__
  return Crails::run_command(Crails::ExecutableCommand{"service", vector<string>{name, "start"}});
#else
  return Crails::run_command(Crails::ExecutableCommand{"systemctl", vector<string>{"start", name}});
#endif
}

bool reload_service(const string& name)
{
#ifdef __FreeBSD__
  return Crails::run_command(Crails::ExecutableCommand{"service", vector<string>{name, "reload"}});
#else
  return Crails::run_command(Crails::ExecutableCommand{"systemctl", vector<string>{"reload", name}});
#endif
}

bool enable_service(const string& name)
{
#ifdef __FreeBSD__
  return Crails::run_command(Crails::ExecutableCommand{"sysrc", vector<string>{name, "_enable=\"YES\""}});
#else
  return Crails::run_command(Crails::ExecutableCommand{"systemctl", vector<string>{"enable", name}});
#endif
}
