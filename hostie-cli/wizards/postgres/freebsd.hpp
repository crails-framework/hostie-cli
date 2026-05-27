#pragma once
#include "wizard.hpp"
#include "../ubuntu.hpp"

class FreeBSDPostgresWizard : public PostgresWizard<UbuntuWizard>
{
public:
  FreeBSDPostgresWizard()
  {
    requirements.push_back("postgresql18-server");
    requirements.push_back("postgresql18-client");
  }

  bool start_service() override
  {
    return Crails::run_command("service postgresql onestart") &&
           Crails::run_command("sysrc postgresql_enable=\"YES\"");
  }
};
