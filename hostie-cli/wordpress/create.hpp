#pragma once
#include "../instance_command.hpp"

class InstanceUser;
class MysqlDatabase;

namespace Wordpress
{
  class CreateCommand : public InstanceCommand
  {
    std::filesystem::path var_directory;
    int state = 0;
  public:
    enum StateFlag
    {
      VarDirectoryCreated = 1,
      DatabaseCreated = 2,
      UserCreated = 4,
      FpmPoolCreated = 8
    };

    std::string_view description() const override
    {
      return "creates a new wordpress site with its own system user and database";
    }

    void options_description(boost::program_options::options_description& options) const override
    {
      using namespace std;
      InstanceCommand::options_description(options);
      options.add_options()
        ("user,u", boost::program_options::value<string>(), "user name")
        ("group,g", boost::program_options::value<string>(), "group name")
        ("runtime-directory,d", boost::program_options::value<string>(), "runtime directory (will store attachments and such)")
        ("wordpress-source,w", boost::program_options::value<string>(), "source directory for wordpress runtime");
    }

    int run() override;

    bool migrate_database();
    bool prepare_runtime_directory(const InstanceUser&);
    bool prepare_wordpress(const InstanceUser&);
    bool generate_wp_config(const InstanceUser&, const MysqlDatabase&);
    bool generate_fpm_pool(const InstanceUser&);
    bool create_user(InstanceUser&);

    std::filesystem::path find_php_fpm_socket_path(const std::filesystem::path& conf_path);
    std::filesystem::path find_wordpress_source() const;
    int cancel(InstanceUser&, MysqlDatabase&);
  };
}
