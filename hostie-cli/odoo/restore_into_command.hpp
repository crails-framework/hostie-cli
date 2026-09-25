#pragma once
#include "create.hpp"
#include "../restore_into_command.hpp"
#include "../databases/postgres.hpp"

namespace Odoo
{
  class RestoreIntoCommand : public ::RestoreIntoCommand<Odoo::CreateCommand, PostgresDatabase>
  {
  public:
    int setup_admin_user(const SystemService&, const PostgresDatabase& database) override
    {
      setup_base_url(database);
      return 0;
    }
  };
}
