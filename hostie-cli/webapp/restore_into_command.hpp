#pragma once
#include "create.hpp"
#include "../databases/database.hpp"
#include <crails/cli/process.hpp>
#include <iostream>

std::filesystem::path crails_backup_bin();

namespace WebApp
{
  // Not built on the ::RestoreIntoCommand<CREATOR, DATABASE> template: it needs
  // the database engine as a compile-time type, whereas it is a runtime choice here.
  class RestoreIntoCommand : public WebApp::CreateCommand
  {
  public:
    void options_description(boost::program_options::options_description& options) const override
    {
      CreateCommand::options_description(options);
      options.add_options()
        ("backup-id,b", boost::program_options::value<unsigned long>(), "id of the backup to seed the instance from")
        ("backup-name", boost::program_options::value<std::string>(), "the name the backup was taken under (in /opt/crails-backup/<n>/). Defaults to --name.");
    }

    bool initialize(int argc, const char** argv) override
    {
      if (!CreateCommand::initialize(argc, argv))
        return false;
      if (!options.count("backup-id"))
      {
        std::cerr << "missing required option --backup-id" << std::endl;
        return false;
      }
      return true;
    }

    bool post_install_actions(const Database& database) override
    {
      using namespace std;
      const string source_name = options.count("backup-name")
        ? options["backup-name"].as<string>()
        : options["name"].as<string>();
      const unsigned long backup_id = options["backup-id"].as<unsigned long>();
      Crails::ExecutableCommand command;

      command.path = crails_backup_bin();
      command
        << "restore" << "-n" << source_name << "--id" << to_string(backup_id)
        << "-f" << ("directory.vardir:" + var_directory.string())
        << "-d" << ("database." + string(database.type()) + '.' + source_name + ':' + database.get_url().to_string());
      // the command line holds the database password: not echoed
      cerr << "+ crails-backup restore -n " << source_name << " --id " << backup_id << endl;
      return Crails::run_command(command);
    }
  };
}
