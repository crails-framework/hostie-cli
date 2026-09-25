#pragma once

std::filesystem::path crails_backup_bin();

template<typename CREATOR, typename DATABASE>
class RestoreIntoCommand : public CREATOR
{
public:
  void options_description(boost::program_options::options_description& options) const override
  {
    CREATOR::options_description(options);
    options.add_options()
      ("backup-id,b", boost::program_options::value<unsigned long>(), "id of the backup to seed the instance from")
      ("backup-name", boost::program_options::value<std::string>(), "the name the backup was taken under, crails-backup looks archives up by this name (in /opt/crails-backup/<name>/)rather than the new instance's own name. Defaults to --name.");
  }

  bool post_install_actions(const DATABASE& database) override
  {
    return restore_backup(database, CREATOR::options["backup-id"].template as<unsigned long>());
  }

  bool restore_backup(const DATABASE& database, unsigned long backup_id) const
  {
    const std::string source_name = CREATOR::options.count("backup-name") ? CREATOR::options["backup-name"].template as<std::string>() : CREATOR::options["name"].template as<std::string>();
    Crails::ExecutableCommand command;

    command.path = crails_backup_bin();
    command << "restore" << "-n" << source_name << "--id" << std::to_string(backup_id);
    apply_restore_targets(command, source_name, database);
    std::cerr << "+ " << command << std::endl;
    return Crails::run_command(command);
  }

  virtual void apply_restore_targets(Crails::ExecutableCommand& command, const std::string& source_name, const DATABASE& database) const
  {
    command << "-f" << ("directory.vardir:" + CREATOR::var_directory.string())
            << "-d" << ("database." + std::string(DATABASE::backup_key) + '.' + source_name + ":" + database.get_url().to_string());
  }
};
