#pragma once
#include "wizard.hpp"
#include "../ubuntu.hpp"

namespace NextCloud
{
  namespace Ubuntu
  {
    class Wizard : public UbuntuWizard, public NextCloud::Wizard
    {
    public:
      int run();
    };
  }
}
