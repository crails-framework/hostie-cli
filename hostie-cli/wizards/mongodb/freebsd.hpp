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
      bool restart_service() override;
      bool prepare_conf();

      std::string get_mongod_conf_path() const override
      {
        return "/usr/local/etc/mongod.conf";
      }
    };
  }
}
