#pragma once
#include "../restore_command.hpp"

namespace WebApp
{
  class RestoreCommand : public ::RestoreCommand
  {
    std::string_view application_type() const override
    {
      return "WebApp";
    }
  };
}
