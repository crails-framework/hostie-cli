#pragma once
#include <vector>
#include "wizard.hpp"
#define FREEBSD_PHP_PREFIX "php85"

class FreeBSDWizard : public WizardBase
{
public:
  std::vector<std::string_view> requirements;
  bool install_requirements();
  bool prepare_conf();
};
