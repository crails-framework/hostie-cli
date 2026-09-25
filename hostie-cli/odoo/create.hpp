#pragma once
#include "../standard_creator.hpp"

class SystemService;
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

    void options_description(boost::program_options::options_description&) const override;
    int run() override;
    int cancel(InstanceUser&, PostgresDatabase&);
    bool generate_odoo_conf(const PostgresDatabase&);
    bool prepare_database(const PostgresDatabase&);
    std::filesystem::path odoo_rc_path() const;
    std::string start_command(const std::filesystem::path& bin, const PostgresDatabase&) const;
    void initialize_admin_password(const std::string& password);
    bool update_admin_password(const PostgresDatabase&) const;
    bool setup_base_url(const PostgresDatabase&) const;
    unsigned short gevent_port() const;

    virtual bool migrate_database() { return true; }
    virtual bool post_install_actions(const PostgresDatabase&) { return true; }
    virtual int setup_admin_user(const SystemService&, const PostgresDatabase&);

  private:
    std::string encoded_admin_password;
  };
}
