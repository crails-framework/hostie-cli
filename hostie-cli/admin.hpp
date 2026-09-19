#pragma once
#include <crails/cli/command_index.hpp>
#include "admin/inventory.hpp"
#include "admin/usage.hpp"

class AdminIndex : public Crails::CommandIndex
{
public:
  AdminIndex()
  {
    add_command("inventory", []() { return std::make_shared<InventoryCommand>(); });
    add_command("usage",     []() { return std::make_shared<UsageCommand>(); });
  }
};
