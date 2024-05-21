#pragma once
#include "../standard_creator.hpp"

class PostgresDatabase;

namespace CrailsCms
{
  class CreateCommand : public StandardCreator
  {
    std::filesystem::path crailscms_bin_dir;
  public:
    std::string_view description() const override
    {
      return "creates a new crails-cms process with its own system user, database, service and network port";
    }

    bool initialize(int argc, const char** argv) override;
    int run() override;

    bool migrate_database(const SystemService& service);
    bool prepare_database(const SystemService&, const PostgresDatabase&);
    int cancel(InstanceUser&, PostgresDatabase&);
  };
}
