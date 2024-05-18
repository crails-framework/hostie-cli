#pragma once
#include "../backup_command.hpp"
#include <filesystem>
#include <sstream>

namespace CrailsCms
{
  class BackupCommand : public ::BackupCommand
  {
  public:
    std::string_view application_type() const override
    {
      return "CrailsCMS";
    }

    void append_add_backup_params(std::ostringstream& command) const override
    {
      using namespace std;
      string database_url = environment.get_variable("DATABASE_URL");
      filesystem::path var_directory = environment.get_variable("VAR_DIRECTORY");
      string schedule = "0 * * *";

      if (options.count("schedule"))
        schedule = options["schedule"].as<string>();
      command
        << "-d " << quoted(database_url)
        << "-f " << var_directory
        << "-s " << quoted(schedule);
    }
  };
}
