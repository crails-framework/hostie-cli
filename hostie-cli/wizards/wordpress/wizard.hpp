#pragma once
#include <vector>
#include <string_view>
#include "../wizard.hpp"

namespace Wordpress
{
  class Wizard : private WizardBase
  {
    HostieVariables store;
  public:
    bool is_installed() const { return store.has_variable("wordpress_src"); }
    bool download_wordpress();
  };
}
