#pragma once
#include <crails/cli/command_index.hpp>
#include "create.hpp"
#include "../environment_command.hpp"
#include "list_command.hpp"
#include "remove_command.hpp"

class NextCloudIndex : public Crails::CommandIndex
{
public:
  NextCloudIndex()
  {
    using namespace NextCloud;
    add_command("list", []() { return std::make_shared<ListNextCloudCommand>(); });
    add_command("config", []() { return std::make_shared<EnvironmentCommand<NextCloud::LiveInstanceCommand>>(); });
    add_command("create", []() { return std::make_shared<CreateCommand>(); });
    add_command("remove", []() { return std::make_shared<RemoveCommand>(); });
    //add_command("backup", []() { return std::make_shared<NextCloud::BackupCommand>(); });
    //add_command("restore", []() { return std::make_shared<NextCloud::RestoreCommand>(); });
  }
};
