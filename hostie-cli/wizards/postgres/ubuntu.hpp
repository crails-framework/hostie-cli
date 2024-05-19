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
};
