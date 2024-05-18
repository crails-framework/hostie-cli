#pragma once
#include "../list_command.hpp"

struct WordpressListTrait
{
  static constexpr const char* name = "Wordpress";
};

typedef ListByTypeCommand<WordpressListTrait> ListWordpressCommand;
