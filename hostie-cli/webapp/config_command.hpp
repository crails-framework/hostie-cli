#pragma once
#include "live_instance_command.hpp"
#include "deployment.hpp"
#include "../environment_command.hpp"
#include <iostream>

namespace WebApp
{
  class ConfigCommand : public EnvironmentCommand<WebApp::LiveInstanceCommand>
  {
  public:
    int run() override
    {
      using namespace std;
      int status = EnvironmentCommand<WebApp::LiveInstanceCommand>::run();
      InstanceEnvironment current;

      current.set_project_name(options["name"].as<string>());
      current.load();
      warn_about_overrides(current, cerr);
      return status;
    }
  };
}
