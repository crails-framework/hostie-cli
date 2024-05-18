#pragma once
#include <filesystem>
#include <string>
#include <string_view>

namespace Crails
{
  long uid_from_username(const std::string&);
  long gid_from_groupname(const std::string&);

  bool chown(const std::filesystem::path&, const std::string_view username);
  bool chown(const std::filesystem::path&, long uid);
  bool chgrp(const std::filesystem::path&, const std::string_view groupname);
  bool chgrp(const std::filesystem::path&, long gid);
}
