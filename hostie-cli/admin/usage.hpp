#pragma once
#include "../instance_command.hpp"

class UsageCommand : public InstanceCommand
{
public:
  std::string_view description() const override
  {
    return "prints as JSON the disk space (in bytes) used by an instance";
  }

  int run() override;
};
