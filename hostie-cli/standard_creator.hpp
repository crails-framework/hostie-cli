#pragma once
#include "./instance_command.hpp"
#include <filesystem>

class SystemService;
class InstanceUser;

class StandardCreator : public InstanceCommand
{
protected:
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

  enum OptionFlags
  {
    WithoutPort = 1,
    WithoutGroup = 2
  };

  virtual void options_description(boost::program_options::options_description& options) const override;
  void custom_options_description(boost::program_options::options_description& options, int flags = 0) const;

  virtual bool prepare_environment_file();
  virtual bool prepare_runtime_directory(const InstanceUser&);
  virtual bool prepare_log_directory(const SystemService&);
  virtual std::filesystem::path get_log_directory() const;
  virtual bool create_user(InstanceUser& user);

  virtual int cancel(InstanceUser&);
};
