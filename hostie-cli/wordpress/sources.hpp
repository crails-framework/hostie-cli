#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace Wordpress
{
  std::filesystem::path source_root();
  std::vector<std::string> installed_versions(const std::filesystem::path& root);
}
