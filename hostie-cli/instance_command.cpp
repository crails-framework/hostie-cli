#include "instance_command.hpp"
#include <sstream>
#include <cstdlib>
#include <iostream>

using namespace std;

bool InstanceCommand::initialize(int argc, const char** argv)
{
  if (Crails::Command::initialize(argc, argv))
  {
    if (options.count("name"))
    {
      environment.set_project_name(options["name"].as<string>());
      return true;
    }
    else
      cerr << "missing required option --name" << endl;
  }
  return false;
}

int InstanceCommand::run_command_as(const std::string& user, const std::string& command)
{
  stringstream sudo_command;
  string str;

  sudo_command
    << "sudo -Eu " << user
    << " env LD_LIBRARY_PATH=\"" << environment.get_variable("LD_LIBRARY_PATH") << '"'
    << " bash -c " << quoted(command);
  str = sudo_command.str();
  cerr << "RUN COMMAND: " << str << endl;
  return std::system(str.c_str());
}

int InstanceCommand::run_command_as_app_user(const std::string& command)
{
  return run_command_as(
    environment.get_variable("APPLICATION_USER"),
    command
  );
}
