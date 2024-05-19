#pragma once
#include "../standard_creator.hpp"

class PostgresDatabase;

namespace CrailsCms
{
  class CreateCommand : public StandardCreator
  {
  public:
    std::string_view description() const override
    {
      return "creates a new crails-cms process with its own system user, database, service and network port";
    }

    int run() override;

    bool migrate_database(const SystemService& service);
    bool prepare_database(const SystemService&, const PostgresDatabase&);
    int cancel(InstanceUser&, PostgresDatabase&);
  };
}
