#pragma once
#include "wizard.hpp"
#include "../ubuntu.hpp"

namespace MongoDB
{
  namespace Ubuntu
  {
    class Wizard : public UbuntuWizard, public MongoDB::Wizard
    {
    public:
      int run();
      bool start_service();
      bool restart_service() override;
    };
  }
}

