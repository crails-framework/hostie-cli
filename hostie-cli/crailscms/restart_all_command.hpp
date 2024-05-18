#pragma once
#include <crails/cli/command.hpp>
#include "../list_command.hpp"

namespace CrailsCms
{
  class RestartAllCommand : public Crails::Command
  {
  public:
    std::string_view description() const override
    {
      return "restarts all the CrailsCMS instances";
    }

    std::vector<std::filesystem::path> collect_environments() const;
    void restart_services(const std::vector<std::filesystem::path>& environments) const;
    void list_status(const std::vector<std::filesystem::path>& environments) const;

    int run() override;
  };
}

