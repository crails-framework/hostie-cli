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
  bool extract_source(const std::string_view url, const std::string_view target_name, const std::filesystem::path& target) const;
  bool install_package_from_urls() const;
  bool apply_web_permissions(const std::filesystem::path& target) const;

  template<typename WIZARD>
  static bool require_wizard()
  {
    WIZARD wizard;

    return wizard.is_installed() || wizard.run() == 0;
  }

  bool add_package_requirement(const std::string_view name, const std::string_view url);

protected:
  std::map<std::string_view, std::string_view> package_urls;
};
