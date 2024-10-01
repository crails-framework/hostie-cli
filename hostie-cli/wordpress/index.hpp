#pragma once
#include <crails/cli/command_index.hpp>
#include "create.hpp"
#include "../environment_command.hpp"
#include "list_command.hpp"
#include "remove_command.hpp"
#include "backup_command.hpp"
#include "restore_command.hpp"
#include "upgrade.hpp"

class WordpressIndex : public Crails::CommandIndex
{
public:
  WordpressIndex()
  {
    using namespace Wordpress;
    add_command("list", []() { return std::make_shared<ListWordpressCommand>(); });
    add_command("config", []() { return std::make_shared<EnvironmentCommand<Wordpress::LiveInstanceCommand>>(); });
    add_command("create", []() { return std::make_shared<CreateCommand>(); });
    add_command("remove", []() { return std::make_shared<RemoveCommand>(); });
    add_command("upgrade", []() { return std::make_shared<Wordpress::MigrateVersionCommand>(); });
    add_command("backup", []() { return std::make_shared<Wordpress::BackupCommand>(); });
    add_command("restore", []() { return std::make_shared<Wordpress::RestoreCommand>(); });
  }
};
