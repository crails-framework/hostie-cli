#pragma once
#include "../list_command.hpp"

struct OdooListTrait
{
  static constexpr const char* name = "Odoo";
};

typedef ListByTypeCommand<OdooListTrait> ListOdooCommand;
