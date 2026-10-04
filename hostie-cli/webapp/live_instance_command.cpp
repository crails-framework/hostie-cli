#include "live_instance_command.hpp"
#include "../service.hpp"
#include <iostream>

bool WebApp::LiveInstanceCommand::restart_service() const
{
  SystemService service;

  service.app_name = environment.get_variable("APPLICATION_NAME");
  if (!service.service_file_exists())
  {
    // crails-deploy creates the unit: before the first deployment there is
    // nothing to restart, and the new environment will be read at first start.
    std::cerr << "no service deployed yet for " << service.app_name
              << ", nothing to restart" << std::endl;
    return true;
  }
  return service.restart();
}
