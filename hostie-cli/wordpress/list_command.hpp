#pragma once
#include "../list_command.hpp"

struct WordpressListTrait
{
  static constexpr const char* name = "Wordpress";
};

typedef ListByTypeCommand<WordpressListTrait> ListWordpressCommand;

namespace Wordpress
{
  class WpListCommand : public ListWordpressCommand
  {
  public:
    void options_description(boost::program_options::options_description& options) const override;

    void display_entry(const std::filesystem::path&) const override;
    void display_with_version(const std::filesystem::path&) const;
  };
}
