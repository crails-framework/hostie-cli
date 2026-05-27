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
#ifdef __FreeBSD__
  return "/usr/local/etc/rc.d/" + app_name;
#else
  return "/etc/systemd/system/" + app_name + ".service";
#endif
}

bool SystemService::start()
{
#ifdef __FreeBSD__
  return Crails::run_command("service " + app_name + " onestart");
#else
  return Crails::run_command("systemctl start " + app_name + ".service");
#endif
}

bool SystemService::restart()
{
#ifdef __FreeBSD__
  return Crails::run_command("service " + app_name + " restart");
#else
  return Crails::run_command("systemctl restart " + app_name + ".service");
#endif
}

bool SystemService::stop()
{
#ifdef __FreeBSD__
  return Crails::run_command("service " + app_name + " stop");
#else
  return Crails::run_command("systemctl stop " + app_name + ".service");
#endif
}

bool SystemService::running()
{
#ifdef __FreeBSD__
  return Crails::run_command("service " + app_name + " status");
#else
  string result;

  Crails::run_command("systemctl status " + app_name + ".service", result);
  return result.find("active (running)") != string::npos;
#endif
}

bool SystemService::status()
{
  return running();
}

bool SystemService::require()
{
  using namespace Crails;
  if (!create_service_file())
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
#ifdef __FreeBSD__
  return true;
#else
  stringstream command;

  if (sudo) command << "sudo ";
  command << "systemctl daemon-reload";
  return Crails::run_command(command.str());
#endif
}

bool SystemService::create_service_file() const
{
  ofstream stream(service_file_path().string());

  if (stream.is_open())
  {
    stream << service_file_contents();
    stream.close();
#ifdef __FreeBSD__
    filesystem::permissions(
      service_file_path(),
      filesystem::perms::owner_all |
      filesystem::perms::group_read |
      filesystem::perms::group_exec,
      filesystem::perm_options::replace
    );
#endif
    return true;
  }
  else
    cerr << "could not create file " << service_file_path().string() << endl;
  return false;
}

std::string SystemService::service_file_contents() const
{
  stringstream stream;

  cout << "Runtime directory: " << runtime_directory << endl;
  cout << "Environ directory: " << environment_path << endl;

#ifdef __FreeBSD__
  //
  // RC Script
  //
  stream
    << "#!/bin/sh\n"
    << "#\n"
    << "# PROVIDE: " << app_name << "\n"
    << "# REQUIRE: NETWORKING\n"
    << "# KEYWORD: shutdown\n"
    << "\n"
    << ". /etc/rc.subr\n"
    << "\n"
    << "name=\"" << app_name << "\"\n"
    << "rcvar=\"" << app_name << "_enable\"\n"
    << "\n"
    << "# Load settings from rc.conf / sysrc\n"
    << "load_rc_config \"${name}\"\n"
    << "\n"
    << ": ${" << app_name << "_enable:=YES}\n"
    << "\n"
    << "pidfile=\"/tmp/hostie-" << app_name << ".pid\"\n"
    << "command=\"" << start_command << "\"\n"
    << "command_args=\"\"\n";
  if (app_user.length() > 0)
    stream << app_name << "_user=\"" << app_user << "\"\n";
  stream << "\nstart_precmd=\"" << app_name << "_prestart\"\n"
         << app_name << "_prestart()\n"
         << "{\n"
         << "  # Export variables defined in the environment file\n"
         << "  set -a\n"
         << "  . \"" << filesystem::canonical(environment_path).string() << "\"\n"
         << "  set +a\n"
         << "}\n";
  if (stop_command.length() > 0)
    stream << "stop_cmd=\"" << stop_command << "\"\n";
  stream
    << "\ncd \"" << filesystem::canonical(runtime_directory).string() << "\" || exit 1\n"
    << "\nrun_rc_command \"$1\"\n";

#else
  //
  // SystemD
  //
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
#endif
  cout << "GENERATING SERVICE FILE CONTENTS: " << endl << stream.str() << endl << endl;
  return stream.str();
}
