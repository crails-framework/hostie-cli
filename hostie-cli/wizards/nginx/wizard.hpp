#pragma once
#include "../wizard.hpp"

namespace Nginx
{
  class Wizard : private WizardBase
  {
  protected:
    HostieVariables store;
  public:
    bool is_installed() const { return store.has_variable("httpd"); }
    int run();
  };
}
