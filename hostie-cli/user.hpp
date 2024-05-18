#pragma once
#include <string>

struct InstanceUser
{
  std::string name, group;

  bool user_exists() const;
  bool group_exists() const;
  bool create();
  bool create_group();
  bool assign_group();
  bool require_user();
  bool require_group();
  bool require();
  bool delete_user();
};
