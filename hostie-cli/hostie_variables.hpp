#pragma once
#include <crails/cli/project_variables.hpp>
#include <memory>

class HostieVariables : public Crails::ProjectVariables
{
public:
  HostieVariables();

  void save();

  static std::unique_ptr<HostieVariables> global;
};
