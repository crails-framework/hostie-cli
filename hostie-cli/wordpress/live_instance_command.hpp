#pragma once
#include "../live_instance_command.hpp"
#include "php.hpp"

namespace Wordpress
{
  class LiveInstanceCommand : public ::LiveInstanceCommand
  {
  public:
    std::string_view application_type() const override
    {
      return "Wordpress";
    }

    std::filesystem::path fpm_pool_path() const
    {
      return Wordpress::fpm_pool_path(environment);
    }
  };
}
