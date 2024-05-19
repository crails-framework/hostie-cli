#pragma once
#include "../live_instance_command.hpp"

namespace Odoo
{
  class LiveInstanceCommand : public ::LiveInstanceCommand
  {
  public:
    std::string_view application_type() const override
    {
      return "Odoo";
    }

    bool restart_service() const override;
  };
}

