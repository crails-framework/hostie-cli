#include "live_instance_command.hpp"
#include <cstdlib>
#include <filesystem>
#include <sstream>

using namespace std;

filesystem::path crails_backup_bin();
  
bool LiveInstanceCommand::initialize(int argc, const char** argv)
{
  if (InstanceCommand::initialize(argc, argv))
  {
    if (filesystem::exists(environment.get_path()))
    {
      environment.load();
      if (environment.get_variable("APPLICATION_TYPE") == application_type())
        return true;
      else
        cerr << "wrong application type " << application_type() << " (expected type was " << environment.get_variable("APPLICATION_TYPE") << ')' << endl;
    }
    else
      cerr << "cannot find file " << environment.get_path() << endl;
  }
  return false;
}

bool LiveInstanceCommand::wipe_backups()
{
  ostringstream command;

  command
   << crails_backup_bin() << " wipe -n "
   << quoted(environment.get_project_name());
  cerr << "+ " << command.str() << endl;
  return system(command.str().c_str()) == 0;
}
