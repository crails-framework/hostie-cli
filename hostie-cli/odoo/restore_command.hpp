#pragma once
#include "../restore_command.hpp"

namespace Odoo
{
  class RestoreCommand : public ::RestoreCommand
  {
    std::string_view application_type() const override
    {
      return "Odoo";
    }
  };
}
