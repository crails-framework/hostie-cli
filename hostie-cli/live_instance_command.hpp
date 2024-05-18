#pragma once
#include "instance_command.hpp"

class LiveInstanceCommand : public InstanceCommand
{
public:
  virtual std::string_view application_type() const = 0;
  virtual bool restart_service() const { return true; }
  bool initialize(int argc, const char** argv) override;
  bool wipe_backups();
};
