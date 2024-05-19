#include "live_instance_command.hpp"
#include "../service.hpp"

bool Odoo::LiveInstanceCommand::restart_service() const
{
  SystemService service;

  service.app_name = environment.get_variable("APPLICATION_NAME");
  return service.restart();
}
