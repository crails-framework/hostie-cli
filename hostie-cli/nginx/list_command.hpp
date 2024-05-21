#pragma once
#include <crails/cli/command.hpp>

namespace Nginx
{
  class ListCommand : public Crails::Command
  {
  public:
    int run() override;
  };
}
