#include <crails/read_file.hpp>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include "standard_creator.hpp"
#include "service.hpp"
#include "user.hpp"

using namespace std;
  
void StandardCreator::options_description(boost::program_options::options_description& options) const
{
  InstanceCommand::options_description(options);
  options.add_options()
    ("port,p", boost::program_options::value<unsigned short>(), "network port to use")
    ("user,u", boost::program_options::value<string>(), "user name")
    ("group,g", boost::program_options::value<string>(), "group name")
    ("runtime-directory,d", boost::program_options::value<string>(), "runtime directory (will store attachments and such)")
    ("env,e", boost::program_options::value<vector<string>>()->multitoken(), "list of environment variables or files");
}

bool StandardCreator::prepare_environment_file()
{
  if (options.count("env"))
  {
    for (const string& param : options["env"].as<vector<string>>())
    {
      string contents;

      if (param.find('=') != string::npos)
        contents = param;
      else if (!Crails::read_file(param, contents))
        cerr << "could not read environment file " << param << endl;
      environment.append(contents);
    }
  }
  return environment.save();
}

bool StandardCreator::prepare_runtime_directory(const InstanceUser& user)
{
  if (options.count("runtime-directory"))
    var_directory = filesystem::weakly_canonical(options["runtime-directory"].as<string>());
  else if (getenv("VAR_DIRECTORY") != 0)
    var_directory = filesystem::weakly_canonical(getenv("VAR_DIRECTORY")) / options["name"].as<string>();
  else
  {
    cerr << "could not deduce runtime directory" << endl;
    return false;
  }

  if (filesystem::is_directory(var_directory) || filesystem::create_directories(var_directory))
    cerr << "using runtime directory: " << var_directory << endl;
  else
  {
    cerr << "could not create runtime directory " << var_directory << endl;
    return false;
  }

  state += VarDirectoryCreated;

  stringstream chown_command, chgrp_command;
  chown_command << "chown -R " << user.name  << ' ' << var_directory;
  chgrp_command << "chown -R " << user.group << ' ' << var_directory;
  cout << "+ " << chown_command.str() << " && " << chgrp_command.str() << endl;
  return system(chown_command.str().c_str()) == 0 && system(chgrp_command.str().c_str()) == 0;
}

filesystem::path StandardCreator::get_log_directory() const
{
  return filesystem::path("/var/log") / environment.get_project_name();
}

bool StandardCreator::prepare_log_directory(const SystemService& service)
{
  filesystem::path directory = get_log_directory();

  if (filesystem::is_directory(directory) || filesystem::create_directories(directory))
  {
    string chown_command = "chown " + service.app_user + ' ' + directory.string();

    cerr << "using log directory: " << directory << endl;
    if (std::system(chown_command.c_str()) != 0)
    {
      cerr << "could not chown log directory " << directory << endl;
      return false;
    }
  }
  else
  {
    cerr << "could not create log directory " << directory << endl;
    return false;
  }
  state += LogDirectoryCreated;
  return true;
}

bool StandardCreator::create_user(InstanceUser& user)
{
  if (user.require())
  {
    state += UserCreated;
    return true;
  }
  return false;
}

int StandardCreator::cancel(InstanceUser& user)
{
  filesystem::remove(environment.get_path());
  if ((state & VarDirectoryCreated) > 0)
    filesystem::remove_all(var_directory);
  if ((state & LogDirectoryCreated) > 0)
    filesystem::remove_all(get_log_directory());
  if ((state & UserCreated) > 0)
    user.delete_user();
  return -1;
}
