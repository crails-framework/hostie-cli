#pragma once
#include "../list_command.hpp"

struct NextCloudListTrait
{
  static constexpr const char* name = "NextCloud";
};

typedef ListByTypeCommand<NextCloudListTrait> ListNextCloudCommand;

