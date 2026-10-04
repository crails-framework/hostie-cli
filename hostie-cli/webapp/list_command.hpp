#pragma once
#include "../list_command.hpp"

struct WebAppListTrait
{
  static constexpr const char* name = "WebApp";
};

typedef ListByTypeCommand<WebAppListTrait> ListWebAppCommand;
