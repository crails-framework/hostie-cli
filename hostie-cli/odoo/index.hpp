#pragma once
#include <crails/cli/command_index.hpp>
#include "create.hpp"
#include "../environment_command.hpp"
#include "list_command.hpp"
#include "remove_command.hpp"
#include "backup_command.hpp"
#include "restore_command.hpp"
#include "status_command.hpp"
//#include "restart_all_command.hpp"

class OdooIndex : public Crails::CommandIndex
{
public:
  OdooIndex()
  {
    using namespace CrailsCms;
    add_command("list", []() { return std::make_shared<ListOdooCommand>(); });
    add_command("config", []() { return std::make_shared<EnvironmentCommand<Odoo::LiveInstanceCommand>>(); });
    add_command("status", []() { return std::make_shared<StatusCommand>(); });
    //add_command("restart-all", []() { return std::make_shared<RestartAllCommand>(); });
    add_command("create", []() { return std::make_shared<CreateCommand>(); });
    add_command("remove", []() { return std::make_shared<RemoveCommand>(); });
    add_command("backup", []() { return std::make_shared<Odoo::BackupCommand>(); });
    add_command("restore", []() { return std::make_shared<Odoo::RestoreCommand>(); });
  }
};

