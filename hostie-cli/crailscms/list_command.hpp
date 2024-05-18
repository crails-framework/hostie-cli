#pragma once
#include "../list_command.hpp"

struct CrailsCmsListTrait
{
  static constexpr const char* name = "CrailsCMS";
};

typedef ListByTypeCommand<CrailsCmsListTrait> ListCrailsCmsCommand;
