#pragma once
#include <vector>
#include "wizard.hpp"

class FedoraWizard : public WizardBase
{
public:
  std::vector<std::string_view> requirements;
  bool install_requirements();
};
