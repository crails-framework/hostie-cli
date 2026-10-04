#pragma once
#include <crails/cli/command_index.hpp>
#include "create.hpp"
#include "../environment_command.hpp"
#include "list_command.hpp"
#include "remove_command.hpp"
#include "status_command.hpp"
#include "config_command.hpp"
#include "deploy_args_command.hpp"
#include "backup_command.hpp"
#include "restore_command.hpp"
#include "restore_into_command.hpp"

class WebAppIndex : public Crails::CommandIndex
{
public:
  WebAppIndex()
  {
    add_command("list",         []() { return std::make_shared<ListWebAppCommand>(); });
    add_command("config",       []() { return std::make_shared<WebApp::ConfigCommand>(); });
    add_command("status",       []() { return std::make_shared<WebApp::StatusCommand>(); });
    add_command("create",       []() { return std::make_shared<WebApp::CreateCommand>(); });
    add_command("remove",       []() { return std::make_shared<WebApp::RemoveCommand>(); });
    add_command("deploy-args",  []() { return std::make_shared<WebApp::DeployArgsCommand>(); });
    add_command("backup",       []() { return std::make_shared<WebApp::BackupCommand>(); });
    add_command("restore",      []() { return std::make_shared<WebApp::RestoreCommand>(); });
    add_command("restore-into", []() { return std::make_shared<WebApp::RestoreIntoCommand>(); });
  }
};
