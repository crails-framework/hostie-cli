#include <crails/read_file.hpp>
#include <crails/utils/join.hpp>
#include <crails/cli/process.hpp>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include "standard_creator.hpp"
#include "service.hpp"
#include "user.hpp"
#include "hostie_variables.hpp"

using namespace std;
  
void StandardCreator::options_description(boost::program_options::options_description& options) const
{
  custom_options_description(options, 0);
}

bool StandardCreator::initialize(int argc, const char** argv)
{
  if (InstanceCommand::initialize(argc, argv))
  {
    if (environment.already_exists())
      cerr << "cannot create instance, name already taken (file " << environment.get_path() << " already exists.)" << endl;
    else
      return true;
  }
  return false;
}

void StandardCreator::custom_options_description(boost::program_options::options_description& options, int flag) const
{
  InstanceCommand::options_description(options);
  if ((flag & WithoutPort) == 0)
  {
    options.add_options()
      ("port,p", boost::program_options::value<unsigned short>(), "network port to use");
  }
  options.add_options()
    ("user,u", boost::program_options::value<string>(), "user name");
  if ((flag & WithoutGroup) == 0)
  {
    options.add_options()
      ("group,g", boost::program_options::value<string>(), "group name");
  }
  options.add_options()
    ("runtime-directory,d", boost::program_options::value<string>(), "runtime directory (will store attachments and such)")
    ("domains,x", boost::program_options::value<vector<string>>()->multitoken(), "list of domain names")
    ("env,e", boost::program_options::value<vector<string>>()->multitoken(), "list of environment variables or files");
}

bool StandardCreator::prepare_environment_file()
{
  if (options.count("domains"))
  {
    environment.set_variable(
      "HOSTIE_DOMAINS",
      Crails::join(options["domains"].as<vector<string>>(), ';')
    );
  }
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
    var_directory = InstanceEnvironment::get_root_path() / options["name"].as<string>();

  if (filesystem::is_directory(var_directory) || filesystem::create_directories(var_directory))
    cerr << "using runtime directory: " << var_directory << endl;
  else
  {
    cerr << "could not create runtime directory " << var_directory << endl;
    return false;
  }

  state += VarDirectoryCreated;

  Crails::ExecutableCommand chown_command{
    "chown", {"-R", user.name, var_directory.string()}
  };
  Crails::ExecutableCommand chgrp_command{
    "chgrp", {"-R", user.group, var_directory.string()}
  };
  cout << "+ " << chown_command << " && " << chgrp_command << endl;
  return Crails::run_command(chown_command) && Crails::run_command(chgrp_command);
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
    cerr << "using log directory: " << directory << endl;
    if (!Crails::run_command({"chown", {service.app_user, directory.string()}}))
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

string StandardCreator::default_admin_login() const
{
  return HostieVariables::global->variable_or("default-admin-login", "admin");
}

string StandardCreator::default_admin_password() const
{
  return HostieVariables::global->variable_or("default-admin-password", "1234");
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
