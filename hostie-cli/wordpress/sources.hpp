#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace Wordpress
{
  std::filesystem::path source_root();
  std::vector<std::string> installed_versions(const std::filesystem::path& root);
  bool is_wordpress_version(const std::string&);
  bool is_complete_source(const std::filesystem::path& source); // check for possibly interrupted install
}
