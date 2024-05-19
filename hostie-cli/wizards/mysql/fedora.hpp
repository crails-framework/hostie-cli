#pragma once
#include "wizard.hpp"
#include "../fedora.hpp"

class FedoraMysqlWizard : public MysqlWizard<FedoraWizard>
{
public:
  FedoraMysqlWizard()
  {
    requirements.push_back("mysql-server");
  }
};
