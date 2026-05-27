#pragma once
#include "wizard.hpp"
#include "../freebsd.hpp"

class FreeBSDTelegrafWizard : public TelegrafWizard<FreeBSDWizard>
{
public:
  FreeBSDTelegrafWizard()
  {
    requirements.push_back("telegraf");
  }

  bool start_service() override
  {
    return Crails::run_command("service telegraf onestart")
        && Crails::run_command("service telegraf enable");
  }

  bool add_telegraf_repository() override
  {
    return true;
  }
};
