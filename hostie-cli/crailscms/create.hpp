#pragma once
#include "../instance_command.hpp"

class SystemService;
class InstanceUser;
class PostgresDatabase;

namespace CrailsCms
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
      LogDirectoryCreated = 8,
    };

    std::string_view description() const override
    {
      return "creates a new crails-cms process with its own system user, database, service and network port";
    }

    void options_description(boost::program_options::options_description& options) const override
    {
      using namespace std;
      InstanceCommand::options_description(options);
      options.add_options()
        ("port,p", boost::program_options::value<unsigned short>(), "network port to use")
        ("user,u", boost::program_options::value<string>(), "user name")
        ("group,g", boost::program_options::value<string>(), "group name")
        ("runtime-directory,d", boost::program_options::value<string>(), "runtime directory (will store attachments and such)")
        ("env,e", boost::program_options::value<vector<string>>()->multitoken(), "list of environment variables or files");
    }

    int run() override;

    bool migrate_database(const SystemService& service);
    bool prepare_environment_file();
    bool prepare_runtime_directory(const InstanceUser&);
    bool prepare_log_directory(const SystemService&);
    bool prepare_database(const SystemService&, const PostgresDatabase&);
    bool create_user(InstanceUser&);
    std::filesystem::path get_log_directory() const;
    int cancel(InstanceUser&, PostgresDatabase&);
  };
}
