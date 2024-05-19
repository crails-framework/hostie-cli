#pragma once
#include "wizard.hpp"
#include "../ubuntu.hpp"

namespace Wordpress
{
  namespace Ubuntu
  {
    class Wizard : public UbuntuWizard, public Wordpress::Wizard
    {
    public:
      int run();
    };
  }
}
