#pragma once
#include "wizard.hpp"
#include "../freebsd.hpp"

class FreeBSDMysqlWizard : public MysqlWizard<FreeBSDWizard>
{
public:
  FreeBSDMysqlWizard()
  {
    service_name = "mysql-server";
    requirements.push_back("mysql97-server");
  }

  bool start_service() override
  {
    return Crails::run_command("service " + service_name + " onestart");
  }

  bool enable_service() override
  {
    return Crails::run_command("sysrc mysql_enable=\"YES\"");
  }
};
