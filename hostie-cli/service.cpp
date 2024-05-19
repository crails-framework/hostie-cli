#include <crails/cli/process.hpp>
#include <crails/cli/terminal.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include "service.hpp"

using namespace std;

extern bool sudo;
  
filesystem::path SystemService::service_file_path() const
{
  return "/etc/systemd/system/" + app_name + ".service";
}

bool SystemService::start()
{
  return Crails::run_command("systemctl start " + app_name + ".service");
}

bool SystemService::restart()
{
  return Crails::run_command("systemctl restart " + app_name + ".service");
}

bool SystemService::stop()
{
  return Crails::run_command("systemctl stop " + app_name + ".service");
}

bool SystemService::running()
{
  string result;

  Crails::run_command("systemctl status " + app_name + ".service", result);
  return result.find("active (running)") != string::npos;
}

bool SystemService::status()
{
  return running();
}

bool SystemService::require()
{
  using namespace Crails;
  if (!service_file_exists() && !create_service_file())
  {
    cerr << "failed to create service file " << service_file_path().string() << endl;
    return false;
  }
  if (!reload_service_files())
  {
    cerr << Terminal::Color::red << Terminal::Background::gray
         << "failed to reload service files"
         << Terminal::reset_color << endl;
    //return false;
  }
  return true;
}

bool SystemService::service_file_exists() const
{
  return Crails::run_command("ls " + service_file_path().string());
}

bool SystemService::reload_service_files() const
{
  stringstream command;

  if (sudo) command << "sudo ";
  command << "systemctl daemon-reload";
  return Crails::run_command(command.str());
}

bool SystemService::create_service_file() const
{
  ofstream stream(service_file_path().string());

  if (stream.is_open())
  {
    stream << service_file_contents();
    return true;
  }
  else
    cerr << "could not create file " << service_file_path().string() << endl;
  return false;
}

std::string SystemService::service_file_contents() const
{
  stringstream stream;

  cout << "GENERATING SERVICE FILE CONTENTS: GROUP = " << app_group << endl;
  cout << "Runtime directory: " << runtime_directory << endl;
  cout << "Environment path: " << environment_path << endl;
  cout << filesystem::exists(runtime_directory) << endl;
  cout << filesystem::exists(environment_path) << endl;
  stream
    << "[Unit]\n"
    << "Description=" << app_name << " CrailsCMS instance managed by cms-farm\n"
    << "After=network.target\n"
    << '\n'
    << "[Service]\n"
    << "WorkingDirectory=" << filesystem::canonical(runtime_directory).string() << '\n'
    << "EnvironmentFile=" << filesystem::canonical(environment_path).string() << '\n';
  if (app_user.length() > 0)
    stream << "User=" << app_user << '\n';
  if (app_group.length() > 0)
    stream << "Group=" << app_group << '\n';
  stream << "ExecStart=" << start_command << '\n';
  if (stop_command.length() > 0)
    stream << "ExecStop=" << stop_command << '\n';
  if (kill_mode.length() > 0)
    stream << "KillMode=" << kill_mode << '\n';
  stream
    << "PIDFile=/tmp/hostie-" << app_name << ".pid\n"
    << "Restart=always\n\n"
    << "[Install]\n"
    << "WantedBy=multi-user.target";
  return stream.str();
}
