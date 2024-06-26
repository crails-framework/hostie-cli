#pragma once
#include "wizard.hpp"
#include "../ubuntu.hpp"

namespace Odoo
{
  namespace Ubuntu
  {
    class Wizard : public UbuntuWizard, public Odoo::Wizard
    {
    public:
      int run();
    };

    class Wizard_2404 : public Wizard
    {
    public:
      int run();
    };
  }
}
