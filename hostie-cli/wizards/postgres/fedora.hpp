#pragma once
#include "wizard.hpp"
#include "../fedora.hpp"

class FedoraPostgresWizard : public PostgresWizard<FedoraWizard>
{
public:
  FedoraPostgresWizard()
  {
    requirements.push_back("postgresql");
    requirements.push_back("postgresql-server");
    requirements.push_back("postgresql-private-libs");
  }

  bool start_service() override
  {
    return Crails::run_command("systemctl enabled postgresql.service") &&
           Crails::run_command("systemctl start postgresql.service");
  }
};
