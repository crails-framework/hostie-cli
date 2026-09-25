#pragma once
#include "../standard_creator.hpp"

class SystemService;
class PostgresDatabase;

namespace CrailsCms
{
  class CreateCommand : public StandardCreator
  {
  protected:
    std::filesystem::path crailscms_bin_dir;
  public:
    std::string_view description() const override
    {
      return "creates a new crails-cms process with its own system user, database, service and network port";
    }

    bool initialize(int argc, const char** argv) override;
    int run() override;

    virtual bool migrate_database();
    bool prepare_database(const SystemService&, const PostgresDatabase&);
    int cancel(InstanceUser&, PostgresDatabase&);

    virtual bool post_install_actions(const PostgresDatabase&) { return true; }
  };
}
