#pragma once
#include "wizard.hpp"
#include "../freebsd.hpp"

namespace Wordpress
{
  namespace FreeBSD
  {
    class Wizard : public FreeBSDWizard, public Wordpress::Wizard
    {
    public:
      int run();
    };
  }
}


