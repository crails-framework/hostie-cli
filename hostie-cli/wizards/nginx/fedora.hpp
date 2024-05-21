#pragma once
#include "wizard.hpp"
#include "../fedora.hpp"

namespace Nginx
{
  namespace Fedora
  {
    class Wizard : public FedoraWizard, public Nginx::Wizard
    {
    public:
      int run();
      bool start_service();
    };
  }
}
