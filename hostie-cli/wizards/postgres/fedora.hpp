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
};
