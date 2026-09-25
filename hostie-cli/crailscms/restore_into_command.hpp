#pragma once
#include "create.hpp"
#include "../restore_into_command.hpp"

namespace CrailsCms
{
  class RestoreIntoCommand : public ::RestoreIntoCommand<CrailsCms::CreateCommand, PostgresDatabase>
  {
  public:
    bool migrate_database() override { return true; }
  };
}
