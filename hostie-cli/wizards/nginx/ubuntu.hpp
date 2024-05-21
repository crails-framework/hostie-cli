#pragma once
#include "wizard.hpp"
#include "../ubuntu.hpp"

namespace Nginx
{
  namespace Ubuntu
  {
    class Wizard : public UbuntuWizard, public Nginx::Wizard
    {
    public:
      int run();
      bool start_service();
    };
  }
}
