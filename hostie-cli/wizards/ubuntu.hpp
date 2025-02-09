#pragma once
#include <vector>
#include "wizard.hpp"

class UbuntuWizard : public WizardBase
{
public:
  std::vector<std::string_view> requirements;
  bool install_requirements();
  bool add_repository(const std::string& name, const std::string_view list, const std::filesystem::path& key);
};
