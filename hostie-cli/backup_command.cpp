#include "backup_command.hpp"
#include <sstream>
#include <filesystem>

using namespace std;

filesystem::path crails_backup_bin()
{
  const char* from_env = getenv("CRAILS_BACKUP_PATH");

  if (!from_env)
  {
    string candidate = Crails::which("crails-backup");
    const char* local_bin = "/usr/local/bin/crails-backup";

    if (!candidate.length())
    {
      if (filesystem::exists(local_bin))
        return filesystem::path(local_bin);
      cerr << "could not locale crails-backup" << endl;
    }
    return candidate;
  }
  return filesystem::path(from_env);
}

void BackupCommand::options_description(boost::program_options::options_description& options) const
{
  ::LiveInstanceCommand::options_description(options);
  options.add_options()
    ("action,a", boost::program_options::value<string>(), "schedule action to perform: add, remove, status, list, or run. Defaults to list.")
    ("schedule,s", boost::program_options::value<string>(), "cron expression describing the periodicity of backups (only used by \"add\")");
}

int BackupCommand::run()
{
  string action = "list";
  Crails::ExecutableCommand command;

  if (options.count("action"))
    action = options["action"].as<string>();
  if (action != "add" && action != "remove" && action != "list" && action != "status" && action != "run")
  {
    cerr << "unknown action " << action << endl;
    return -1;
  }
  command.path = crails_backup_bin();
  if (action == "run")
  {
    command << "backup" << "-n" << environment.get_project_name();
    append_backup_source_params(command);
  }
  else
  {
    command << action << "-n" << environment.get_project_name();
    if (action == "add")
    {
      append_backup_source_params(command);
      command << "-s" << (options.count("schedule") ? options["schedule"].as<string>() : default_backup_schedule());
    }
  }
  return Crails::run_command(command);
}
