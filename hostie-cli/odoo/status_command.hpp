#pragma once
#include "live_instance_command.hpp"
#include <iostream>

namespace Odoo
{
  class StatusCommand : public Odoo::LiveInstanceCommand
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
      if (service.status())
        cout << "running" << endl;
      else
        cout << "stopped" << endl;
      return 0;
    }
  };
}
