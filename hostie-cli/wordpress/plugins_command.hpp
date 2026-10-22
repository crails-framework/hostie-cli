#pragma once
#include "live_instance_command.hpp"

namespace Wordpress
{
  // Prints, as JSON, what an instance runs: its version of WordPress, and the
  // plugins it uses (global and local ones). This is read by cms-farm.
  //
  //   {
  //     "instance": "my_blog",
  //     "wordpress": {
  //       "version": "6.6.2",                  the version the instance runs
  //       "source": "/opt/hostie/wordpress-src/6.6.2",
  //       "shared_by": 4,                      instances running that version: what a global plugin reaches
  //       "installed": ["6.6.2", "6.8.3"],     versions available on this machine
  //       "latest": "6.8.3"                    --check only: wordpress.org's latest, absent if unknown
  //     },
  //     "update_check": "ok",                  --check only: "ok", or "failed" if wordpress.org was out of reach
  //     "plugins": [{
  //       "slug": "akismet", "scope": "global", "name": "Akismet", "version": "5.3",
  //       "linked": 1, "broken": 0, "overrides_global": 0,
  //       "latest": "5.3.5", "latest_requires": "5.8"   --check only: absent for plugins wordpress.org doesn't know
  //     }]
  //   }
  //
  // Booleans are 0 or 1, and nothing is ever null: keys that have no value are absent.
  class PluginsCommand : public Wordpress::LiveInstanceCommand
  {
  public:
    std::string_view description() const override
    {
      return "prints as JSON the version of wordpress and the plugins (global and local) an instance uses";
    }

    void options_description(boost::program_options::options_description& options) const override
    {
      LiveInstanceCommand::options_description(options);
      options.add_options()
        ("check", "look up the latest versions on wordpress.org");
    }

    int run() override;
  };
}
