#pragma once
#include "../list_command.hpp"

struct HtmlListTrait
{
  static constexpr const char* name = "HTML";
};

namespace Html
{
  typedef ListByTypeCommand<HtmlListTrait> ListCommand;
}
