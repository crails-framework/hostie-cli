#pragma once
#include "../phpfpm_creator.hpp"

class InstanceUser;
class MysqlDatabase;

namespace NextCloud
{
  class CreateCommand : public PhpFpmCreator
  {
  public:
    std::string_view description() const override
    {
      return "creates a new nextcloud instance with its own system user and database";
    }

    void options_description(boost::program_options::options_description& options) const override
    {
      using namespace std;
      PhpFpmCreator::options_description(options);
      options.add_options()
        ("skip-install", "skip nextcloud installer")
        ("nextcloud-source,s", boost::program_options::value<string>(), "source directory for nextcloud runtime");
    }

    int run() override;

    void append_custom_fpm_pool_settings(std::ostream&) override;
    bool prepare_nextcloud(const InstanceUser&);
    bool install_nextcloud(const InstanceUser&, const MysqlDatabase&);
    bool post_install_nextcloud(const InstanceUser&);
    bool configure_overwrite_cli_url(const InstanceUser&, const std::string&);
    bool configure_trusted_domains(const InstanceUser&, const std::vector<std::string>&);

    bool run_occ_command(const InstanceUser&, const std::string&);

    std::filesystem::path find_nextcloud_source() const;
    std::string find_nextcloud_version() const;
    int cancel(InstanceUser&, MysqlDatabase&);
  };
}

