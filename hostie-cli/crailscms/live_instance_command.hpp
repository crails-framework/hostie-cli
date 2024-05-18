#pragma once
#include "../live_instance_command.hpp"

namespace CrailsCms
{
  class LiveInstanceCommand : public ::LiveInstanceCommand
  {
  public:
    std::string_view application_type() const override
    {
      return "CrailsCMS";
    }

    bool restart_service() const override;
  };
}
