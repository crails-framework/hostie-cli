#pragma once
#include "live_instance_command.hpp"
#include "deployment.hpp"
#include <iostream>

namespace WebApp
{
  class DeployArgsCommand : public WebApp::LiveInstanceCommand
  {
  public:
    std::string_view description() const override
    {
      return "prints the crails-deploy options matching an instance (eg: crails-deploy ... $(ssh server hostie-cli webapp deploy-args -n foo))";
    }

    void options_description(boost::program_options::options_description& options) const override
    {
      LiveInstanceCommand::options_description(options);
      options.add_options()
        ("example,x", "prints a complete, annotated crails-deploy command instead of just the options");
    }

    int run() override
    {
      using namespace std;

      if (options.count("example"))
        cout << deploy_example(environment) << endl;
      else
        cout << deploy_arguments_line(environment) << endl;
      return 0;
    }
  };
}
