#pragma once
#include "live_instance_command.hpp"

class BackupCommand : public ::LiveInstanceCommand
{
public:
  std::string_view description() const override
  {
    return "schedule backups using crails-backup";
  }

  int run() override;
  void options_description(boost::program_options::options_description& options) const override;

protected:
  virtual void append_add_backup_params(std::ostringstream& command) const = 0;
};
