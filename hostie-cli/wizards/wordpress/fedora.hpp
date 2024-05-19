#pragma once
#include "wizard.hpp"
#include "../fedora.hpp"

namespace Wordpress
{
  namespace Fedora
  {
    class Wizard : public FedoraWizard, public Wordpress::Wizard
    {
    public:
      int run();
    };
  }
}

