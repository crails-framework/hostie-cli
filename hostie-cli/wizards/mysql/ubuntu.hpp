#pragma once
#include "wizard.hpp"
#include "../ubuntu.hpp"

class UbuntuMysqlWizard : public MysqlWizard<UbuntuWizard>
{
public:
  UbuntuMysqlWizard()
  {
    service_name = "mysql.service";
    requirements.push_back("mysql-server");
  }

  bool start_service() override
  {
    return Crails::run_command("systemctl enable " + service_name) &&
           Crails::run_command("systemctl start " + service_name);
  }
};
