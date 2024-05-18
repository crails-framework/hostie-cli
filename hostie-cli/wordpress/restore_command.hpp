#pragma once
#include "../restore_command.hpp"

namespace Wordpress
{
  class RestoreCommand : public ::RestoreCommand
  {
    std::string_view application_type() const override
    {
      return "Wordpress";
    }
  };
}
