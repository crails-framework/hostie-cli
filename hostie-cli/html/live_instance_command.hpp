#pragma once
#include "../live_instance_command.hpp"

namespace Html
{
  class LiveInstanceCommand : public ::LiveInstanceCommand
  {
  public:
    std::string_view application_type() const override
    {
      return "HTML";
    }
  };
}
