#pragma once
#include "../phpfpm_creator.hpp"

class InstanceUser;
class MysqlDatabase;

namespace Wordpress
{
  class CreateCommand : public PhpFpmCreator
  {
  public:
    std::string_view description() const override
    {
      return "creates a new wordpress site with its own system user and database";
    }

    void options_description(boost::program_options::options_description& options) const override
    {
      using namespace std;
      PhpFpmCreator::options_description(options);
      options.add_options()
        ("wordpress-source,s", boost::program_options::value<string>(), "source directory for wordpress runtime");
    }

    int run() override;

    bool migrate_database();
    bool prepare_wordpress(const InstanceUser&);
    bool generate_wp_config(const InstanceUser&, const MysqlDatabase&);

    std::filesystem::path find_wordpress_source() const;
    int cancel(InstanceUser&, MysqlDatabase&);
  };
}
