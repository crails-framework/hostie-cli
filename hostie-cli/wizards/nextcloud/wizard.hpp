#pragma once
#include "../wizard.hpp"

namespace NextCloud
{
  class Wizard : private WizardBase
  {
  public:
    bool is_installed() const { return HostieVariables::global->has_variable("nextcloud-source"); }

    bool download_nextcloud();
    bool generate_version_retriever();
  };
}
