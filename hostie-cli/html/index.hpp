#pragma once
#include <crails/cli/command_index.hpp>
#include "create.hpp"
#include "list_command.hpp"
#include "remove_command.hpp"
#include "deploy_command.hpp"

class HtmlIndex : public Crails::CommandIndex
{
public:
  HtmlIndex()
  {
    using namespace Html;
    add_command("list", []() { return std::make_shared<Html::ListCommand>(); });
    add_command("create", []() { return std::make_shared<CreateCommand>(); });
    add_command("remove", []() { return std::make_shared<RemoveCommand>(); });
    add_command("desploy", []() { return std::make_shared<DeployCommand>(); });
  }
};
