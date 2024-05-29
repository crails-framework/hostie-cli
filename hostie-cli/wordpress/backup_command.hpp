#pragma once
#include "../backup_command.hpp"
#include <filesystem>
#include <sstream>

namespace Wordpress
{
  class BackupCommand : public ::BackupCommand
  {
  public:
    std::string_view application_type() const override
    {
      return "Wordpress";
    }

    void append_add_backup_params(Crails::ExecutableCommand& command) const override
    {
      using namespace std;
      string database_url = environment.get_variable("DATABASE_URL");
      filesystem::path var_directory = environment.get_variable("VAR_DIRECTORY");
      string schedule = "0 * * *";

      if (options.count("schedule"))
        schedule = options["schedule"].as<string>();
      command
        << "-d" << database_url
        << "-f" << (var_directory / "wp-content").string()
        << "-s" << schedule;
    }
  };
}
