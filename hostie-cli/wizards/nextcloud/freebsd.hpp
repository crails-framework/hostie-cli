#pragma once
#include "wizard.hpp"
#include "../freebsd.hpp"

namespace NextCloud
{
  namespace FreeBSD
  {
    class Wizard : public FreeBSDWizard, public NextCloud::Wizard
    {
    public:
      int run();
    };
  }
}
