#pragma once
#include "live_instance_command.hpp"
#include "deployment.hpp"
#include "../service.hpp"
#include <iostream>

namespace WebApp
{
  class StatusCommand : public WebApp::LiveInstanceCommand
  {
  public:
    std::string_view description() const override
    {
      return "check the status of an instance and prints it to stdout";
    }

    int run() override
    {
      using namespace std;
      SystemService service;

      service.app_name = options["name"].as<string>();
      // an instance that was never deployed is simply reported as stopped
      if (service.status())
        cout << "running" << endl;
      else
        cout << "stopped" << endl;
      warn_about_overrides(environment, cerr);
      return 0;
    }
  };
}
