#pragma once
#include "wizard.hpp"
#include "../fedora.hpp"

namespace Odoo
{
  namespace Fedora
  {
    class Wizard : public FedoraWizard, public Odoo::Wizard
    {
    public:
      int run();
    };
  }
}
