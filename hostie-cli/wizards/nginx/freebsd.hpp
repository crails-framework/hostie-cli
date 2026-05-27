#pragma once
#include "wizard.hpp"
#include "../freebsd.hpp"

namespace Nginx
{
  namespace FreeBSD
  {
    class Wizard : public FreeBSDWizard, public Nginx::Wizard
    {
    public:
      int run();
      bool start_service();
      bool prepare_conf();
    };
  }
}
