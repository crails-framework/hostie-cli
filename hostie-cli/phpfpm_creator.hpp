#pragma once
#include "./standard_creator.hpp"
#include <filesystem>
#include <ostream>

class SystemService;
class InstanceUser;

class PhpFpmCreator : public StandardCreator
{
public:
  enum CustomStateFlag
  {
    FpmPoolCreated = 256
  };

  virtual void options_description(boost::program_options::options_description& options) const override
  {
    custom_options_description(
      options,
      StandardCreator::WithoutPort |
      StandardCreator::WithoutGroup
    );
  }

  std::filesystem::path find_php_source(const std::string& name, const char* env_key) const;
  std::filesystem::path find_php_fpm_socket_path(const std::filesystem::path& fpm_conf_path);
  bool generate_fpm_pool(const InstanceUser&);
  static bool restart_php_fpm();
  virtual void append_custom_fpm_pool_settings(std::ostream&) {}

  virtual int cancel(InstanceUser&) override;
};
