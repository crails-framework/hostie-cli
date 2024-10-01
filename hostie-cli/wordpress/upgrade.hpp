#pragma once
#include "live_instance_command.hpp"

struct InstanceUser;

namespace Wordpress
{
  class MigrateVersionCommand : public Wordpress::LiveInstanceCommand
  {
    std::filesystem::path var_directory;
  public:
    std::string_view description() const override
    {
      return "migrate an instance from an installed version of wordpress to another";
    }

    void options_description(boost::program_options::options_description&) const override;
    int run() override;

  private:
    bool unprepare_wordpress(const std::filesystem::path&, const InstanceUser&);
    bool prepare_wordpress(const std::filesystem::path&, const InstanceUser&);
  };
}
