#pragma once
#include "list_command.hpp"

class InventoryCommand : public ListCommand
{
  std::string_view description() const override
  {
    return "listing of all the configured instances";
  }

  int run() override;
};
