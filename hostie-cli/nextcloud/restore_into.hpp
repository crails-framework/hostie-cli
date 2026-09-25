#pragma once
#include "create.hpp"
#include "../restore_into_command.hpp"

namespace NextCloud
{
  class RestoreIntoCommand : public ::RestoreIntoCommand<NextCloud::CreateCommand, MysqlDatabase>
  {
  public:
    bool install_nextcloud(const InstanceUser& user, const MysqlDatabase& database) override
    {
      // TODO there's probably something to do here, to potentially update database honame and all
      // Perhaps NextCloud documentation knows what to do ?
      return true;
    }

    void apply_restore_targets(Crails::ExecutableCommand& command, const std::string& source_name, const DATABASE& database) const
    {
      // TODO almost certainly something to do here, but it has to be synced with the backup command
      // which is not yet implemented
      command << "-f" << ("directory.vardir:" + var_directory.string())
              << "-d" << ("database." + std::string(DATABASE::backup_key) + '.' + source_name + ":" + database.get_url().to_string());
    }
  };
}
