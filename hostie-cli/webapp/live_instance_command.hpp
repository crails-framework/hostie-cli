#pragma once
#include "../live_instance_command.hpp"

namespace WebApp
{
  class LiveInstanceCommand : public ::LiveInstanceCommand
  {
  public:
    std::string_view application_type() const override
    {
      return "WebApp";
    }

    bool restart_service() const override;
  };
}
