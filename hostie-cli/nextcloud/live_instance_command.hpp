#pragma once
#include "../live_instance_command.hpp"
#include "../php.hpp"

namespace NextCloud
{
  class LiveInstanceCommand : public ::LiveInstanceCommand
  {
  public:
    std::string_view application_type() const override
    {
      return "NextCloud";
    }

    std::filesystem::path fpm_pool_path() const
    {
      return ::fpm_pool_path(environment);
    }
  };
}

