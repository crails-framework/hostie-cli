#pragma once
#include <crails/cli/process.hpp>
#include <crails/cli/filesystem.hpp>
#include <iostream>
#include "../../hostie_variables.hpp"

std::string telegraf_conf();

template<typename WIZARD_BASE>
class TelegrafWizard : public WIZARD_BASE
{
  typedef WIZARD_BASE Super;
  HostieVariables& store;
public:
  TelegrafWizard() : store(*HostieVariables::global) {}

  bool is_installed() const { return store.has_variable("telegraf"); }

  int run()
  {
    using namespace std;

    if (add_telegraf_repository() && Super::install_requirements())
    {
      if (Crails::write_file("telegraf", "/etc/telegraf/telegraf.conf", telegraf_conf()))
      {
        store.variable("telegraf", "1");
        if (start_service())
        {
          store.save();
          return 0;
        }
        else
          cerr << "failed to start service" << endl;
      }
      else
        cerr << "failed to generate config file" << endl;
    }
    return -1;
  }

  virtual bool start_service() = 0;
  virtual bool add_telegraf_repository() = 0;
};
