#pragma once
#include <vector>
#include <string_view>
#include "../wizard.hpp"

namespace Wordpress
{
  class Wizard : private WizardBase
  {
  protected:
    HostieVariables store;
  public:
    bool is_installed() const { return store.has_variable("wordpress-source"); }
    bool download_wordpress();
    bool apply_phpfpm_permissions(const std::filesystem::path&);
  };
}
