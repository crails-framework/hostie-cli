#pragma once
#include <vector>
#include <string_view>
#include "../wizard.hpp"

namespace Wordpress
{
  class Wizard : private WizardBase
  {
  public:
    bool is_installed() const { return HostieVariables::global->has_variable("wordpress-source"); }
    bool download_wordpress();
  };
}
