#pragma once
#include "../standard_creator.hpp"

class PostgresDatabase;

namespace Odoo
{
  class CreateCommand : public StandardCreator
  {
  public:
    std::string_view description() const override
    {
      return "creates a new odoo instance with its own process and database";
    }

    int run() override;
    int cancel(InstanceUser&, PostgresDatabase&);
    bool generate_odoo_conf(const PostgresDatabase&);
    bool prepare_database(const PostgresDatabase&);
    std::filesystem::path odoo_rc_path() const;

  private:
    std::string encoded_admin_password;
  };
}
