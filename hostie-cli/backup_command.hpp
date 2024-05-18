#pragma once
#include "live_instance_command.hpp"

class BackupCommand : public ::LiveInstanceCommand
{
public:
  std::string_view description() const override
  {
    return "schedule backups using crails-backup";
  }

  void options_description(boost::program_options::options_description& options) const override
  {
    using namespace std;
    ::LiveInstanceCommand::options_description(options);
    options.add_options()
      ("action,a", boost::program_options::value<string>(), "schedule action to perform: add, remove, status or list. Defaults to list.")
      ("schedule,s", boost::program_options::value<string>(), "cron expression describing the periodicity of backups");
  }

  int run() override
  {
    using namespace std;
    string action = "list";
    ostringstream command;

    if (options.count("action"))
      action = options["action"].as<string>();
    if (action != "add" && action != "remove" && action != "list" && action != "status")
    {
      cerr << "unknown action " << action << endl;
      return -1;
    }
    command
      << "/usr/local/bin/crails-backup " << action << ' '
      << "-n " << quoted(environment.get_project_name());
    if (action == "add")
      append_add_backup_params(command);
    return std::system(command.str().c_str()) == 0;
  }

protected:
  virtual void append_add_backup_params(std::ostringstream& command) const = 0;
};
