#include "user.hpp"
#include <iostream>
#include <sstream>
#include <crails/cli/process.hpp>

using namespace std;

extern bool sudo;

bool InstanceUser::user_exists() const
{
  return Crails::run_command("id -u \"" + name + '"');
}

bool InstanceUser::group_exists() const
{
  return Crails::run_command("grep \"^" + group + ":\" /etc/group");
}

bool InstanceUser::delete_user()
{
  if (user_exists())
    return Crails::run_command("userdel \"" + name + '"');
  return true;
}

bool InstanceUser::create()
{
  stringstream command;
  if (sudo) command << "sudo ";
  command << "useradd --no-create-home \"" << name << '"'
          << " -g \"" << group << '"';
  return Crails::run_command(command.str());
}

bool InstanceUser::create_group()
{
  stringstream command;
  if (sudo) command << "sudo ";
  command << "groupadd \"" << group << '"';
  return Crails::run_command(command.str());
}

bool InstanceUser::assign_group()
{
  stringstream command;
  if (sudo) command << "sudo ";
  command << "usermod -g \"" << group << '"'
          << " \"" << name << '"';
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
