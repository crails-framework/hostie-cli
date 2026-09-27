#pragma once
#include <crails/cli/command.hpp>

class CapabilitiesCommand : public Crails::Command
{
public:
  std::string_view description() const override
  {
    return "lists the installed wizards";
  }

  int run() override;
};
