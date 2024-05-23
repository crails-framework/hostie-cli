#pragma once
#include "wizard.hpp"
#include "../ubuntu.hpp"

class UbuntuPostgresWizard : public PostgresWizard<UbuntuWizard>
{
public:
  UbuntuPostgresWizard()
  {
    requirements.push_back("postgresql");
  }

  bool start_service() override
  {
    return Crails::run_command("systemctl enable postgresql.service") &&
           Crails::run_command("systemctl start postgresql.service");
  }
};
