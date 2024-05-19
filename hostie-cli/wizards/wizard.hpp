#pragma once
#include <string_view>
#include <filesystem>
#include "../hostie_variables.hpp"

class WizardBase
{
public:
  static char system_matches(const std::string_view distribution, const std::string_view version);
  bool download_file(const std::string_view url, const std::filesystem::path& target) const;
  bool download_tar_archive(const std::string_view url) const;
  bool download_zip_archive(const std::string_view url) const;

  template<typename WIZARD>
  static bool require_wizard()
  {
    WIZARD wizard;

    return wizard.is_installed() || wizard.run() == 0;
  }
};
