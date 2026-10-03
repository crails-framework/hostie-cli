#pragma once
#include "wizard.hpp"
#include "../freebsd.hpp"

namespace MongoDB
{
  namespace FreeBSD
  {
    class Wizard : public FreeBSDWizard, public MongoDB::Wizard
    {
    public:
      int run();
      bool start_service();
      bool prepare_conf();
    };
  }
}
