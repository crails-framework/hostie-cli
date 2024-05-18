#pragma once
#include <crails/cli/command.hpp>
#include <filesystem>

class ListCommand : public Crails::Command
{
public:
  std::string_view description() const override
  {
    return "displays a list of configured instances";
  }

  virtual bool should_display_entry(const std::filesystem::path& filepath) const
  {
    return true;
  }

  int run() override;

  static std::vector<std::filesystem::path> instance_environments();
  static bool has_environment_type(const std::filesystem::path&, const std::string_view);
};

template<typename TRAIT>
class ListByTypeCommand : public ListCommand
{
  bool should_display_entry(const std::filesystem::path& filepath) const override
  {
    return ListCommand::should_display_entry(filepath)
      && has_environment_type(filepath, TRAIT::name);
  }
};
