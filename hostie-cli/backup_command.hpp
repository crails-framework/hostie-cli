#pragma once
#include "live_instance_command.hpp"
#include <crails/cli/process.hpp>

class BackupCommand : public ::LiveInstanceCommand
{
public:
  std::string_view description() const override
  {
    return "schedule or run a backup using crails-backup";
  }

  int run() override;
  void options_description(boost::program_options::options_description& options) const override;

protected:
  virtual void append_backup_source_params(Crails::ExecutableCommand& command) const = 0;
  virtual std::string default_backup_schedule() const { return "0 * * *"; }
};
