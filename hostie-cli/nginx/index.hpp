#pragma once
#include <crails/cli/command_index.hpp>
#include "site.hpp"
#include "certify.hpp"
#include "certificate_status.hpp"
#include "list_command.hpp"
#include "remove_command.hpp"

namespace Nginx
{
  class IndexCommand : public Crails::CommandIndex
  {
  public:
    IndexCommand()
    {
      add_command("list",         []() { return std::make_shared<ListCommand>(); });
      add_command("configure",    []() { return std::make_shared<ConfigureSite>(); });
      add_command("certify",      []() { return std::make_shared<Certify>(); });
      add_command("certificates", []() { return std::make_shared<CertificateStatus>(); });
      add_command("remove",       []() { return std::make_shared<RemoveCommand>(); });
    }
  };
}
