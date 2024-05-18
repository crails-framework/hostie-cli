#pragma once
#include <crails/cli/command.hpp>
#include <iostream>
#include "environment.hpp"

class InstanceCommand : public Crails::Command
{
protected:
  InstanceEnvironment environment;
public:
  virtual void options_description(boost::program_options::options_description& options) const override
  {
    using namespace std;
    options.add_options()
      ("name,n", boost::program_options::value<string>(), "service name");
  }

  virtual bool initialize(int argc, const char** argv) override;

  int run_command_as(const std::string& user, const std::string& command);
  int run_command_as_app_user(const std::string& command);
};
