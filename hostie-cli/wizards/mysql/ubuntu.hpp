#pragma once
#include "wizard.hpp"
#include "../ubuntu.hpp"

class UbuntuMysqlWizard : public MysqlWizard<UbuntuWizard>
{
public:
  UbuntuMysqlWizard()
  {
    requirements.push_back("mysql-server");
  }
};
