#pragma once
#include <crails/cli/command.hpp>

namespace Wordpress
{
  // Makes a version of WordPress available on the machine (the same download
  // `hostie-cli wizard wordpress` does, without anything else the wizard
  // does): instances can then be moved to it with `wordpress upgrade`.
  // Does nothing if that version is already there.
  //
  // Like the wizard, it makes that version the one new instances get.
  //
  // Exit status: 0 available, 2 not a version number, 4 the download failed.
  class SourceCommand : public Crails::Command
  {
  public:
    std::string_view description() const override
    {
      return "installs the sources of a version of wordpress, so that instances can be upgraded to it";
    }

    void options_description(boost::program_options::options_description& options) const override
    {
      options.add_options()
        ("version,v", boost::program_options::value<std::string>(), "version number (eg: 6.8.3)");
    }

    int run() override;
  };
}
