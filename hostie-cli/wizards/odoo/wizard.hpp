#pragma once
#include "../wizard.hpp"

namespace Odoo
{
  class Wizard : public WizardBase
  {
    HostieVariables store;
  public:
    bool is_installed() const { return store.has_variable("odoo"); }
    int run()
    {
      store.variable("odoo", "1");
      store.save();
      return 0;
    }
  };
}
