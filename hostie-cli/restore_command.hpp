#pragma once
#include "live_instance_command.hpp"
#include <crails/cli/process.hpp>
#include <filesystem>
#include <iostream>

std::filesystem::path crails_backup_bin();

class RestoreCommand : public ::LiveInstanceCommand
{
public:
  std::string_view description() const override
  {
    return "restore a backup using crails-backup";
  }

  void options_description(boost::program_options::options_description& options) const override
  {
    using namespace std;
    InstanceCommand::options_description(options);
    options.add_options()
      ("id,i", boost::program_options::value<string>(), "id of the backup to restore");
  }

  int run() override
  {
    using namespace std;
    Crails::ExecutableCommand command;

    if (!options.count("id"))
    {
      cerr << "missing required option --id" << endl;
      return -1;
    }
    command.path = crails_backup_bin();
    command << "restore" << "-n" << environment.get_project_name()
            << "--id" << options["id"].as<string>();
    cout << "+ " << command << endl;
    return Crails::run_command(command) ? 0 : 1;
  }
};
