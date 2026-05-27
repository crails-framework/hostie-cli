#include "user.hpp"
#include <iostream>
#include <sstream>
#include <crails/cli/process.hpp>

using namespace std;

extern bool sudo;

bool InstanceUser::user_exists() const
{
#ifdef __FreeBSD__
  return Crails::run_command({"pw", {"user", "show",  "-n", name}});
#else
  return Crails::run_command({"id", {"-u", name}});
#endif
}

bool InstanceUser::group_exists() const
{
#ifdef __FreeBSD__
  return Crails::run_command({"pw", {"group", "show",  "-n", group}});
#else
  return Crails::run_command({"grep", {'^' + group + ':', "/etc/group"}});
#endif
}

bool InstanceUser::delete_user()
{
  if (user_exists())
  {
#ifdef __FreeBSD__
    return Crails::run_command({"pw", {"user", "del", "-n", name}});
#else
    return Crails::run_command({"userdel", {name}});
#endif
  }
  return true;
}

bool InstanceUser::create()
{
  stringstream command;
  if (sudo) command << "sudo ";
#ifdef __FreeBSD__
  command << "pw user add \"" << name << '"'
          << " -g \"" << group << '"';
#else
  command << "useradd --no-create-home \"" << name << '"'
          << " -g \"" << group << '"';
#endif
  return Crails::run_command(command.str());
}

bool InstanceUser::create_group()
{
  stringstream command;
  if (sudo) command << "sudo ";
#ifdef __FreeBSD__
  command << "pw group add \"" << group << '"';
#else
  command << "groupadd \"" << group << '"';
#endif
  return Crails::run_command(command.str());
}

bool InstanceUser::assign_group()
{
  stringstream command;
  if (sudo) command << "sudo ";
#ifdef __FreeBSD__
  command << "pw user mod -n \"" << name << '"'
          << " -g \"" << group << '"';
#else
  command << "usermod -g \"" << group << '"'
          << " \"" << name << '"';
#endif
  return Crails::run_command(command.str());
}

bool InstanceUser::require_user()
{
  if (!user_exists() && !create())
  {
    cerr << "could not create user " << name << endl;
    return false;
  }
  return true;
}

bool InstanceUser::require_group()
{
  if (!group_exists() && !create_group())
  {
    cerr << "could not create group " << group << endl;
    return false;
  }
  return true;
}

bool InstanceUser::require()
{
  if (require_group() && require_user())
  {
    if (!assign_group())
    {
      cerr << "failed to assign group" << endl;
      return false;
    }
    return true;
  }
  return false;
}
