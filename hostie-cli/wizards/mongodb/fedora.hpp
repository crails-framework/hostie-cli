#pragma once
#include "wizard.hpp"
#include "../fedora.hpp"

namespace MongoDB
{
  namespace Fedora
  {
    class Wizard : public FedoraWizard, public MongoDB::Wizard
    {
    public:
      int run();
      bool start_service();
    };
  }
}


