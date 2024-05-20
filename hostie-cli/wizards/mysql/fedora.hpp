#pragma once
#include "wizard.hpp"
#include "../fedora.hpp"

class FedoraMysqlWizard : public MysqlWizard<FedoraWizard>
{
public:
  FedoraMysqlWizard()
  {
    service_name = "mysqld";
    requirements.push_back("mysql-server");
  }

  bool start_service() override
  {
    return Crails::run_command("systemctl enable " + service_name) &&
           Crails::run_command("systemctl start " + service_name);
  }
};
