#pragma once
#include <crails/cli/command.hpp>
#include <iostream>
#include "live_instance_command.hpp"
#include "environment.hpp"

template<typename COMMAND_PARENT>
class EnvironmentCommand : public COMMAND_PARENT
{
  InstanceEnvironment environment;
public:
  std::string_view description() const override
  {
    return "displays and update the content of an instance's environment.";
  }

  void options_description(boost::program_options::options_description& options) const override
  {
    using namespace std;
    options.add_options()
      ("name,n", boost::program_options::value<string>(), "instance name")
      ("get,g", boost::program_options::value<string>(), "display a variable's value by name (eg: --get var)")
      ("set,s", boost::program_options::value<string>(), "updates a variable's value by name (eg: --set var=value)");
  }

  int run() override
  {
    using namespace std;
    environment.set_project_name(COMMAND_PARENT::options["name"].template as<string>());
    environment.load();
    if (COMMAND_PARENT::options.count("get"))
    {
      auto variable = COMMAND_PARENT::options["get"].template as<std::string>();
      auto value = environment.get_variable(variable);
      cout << value << endl;
      return 0;
    }
    else if (COMMAND_PARENT::options.count("set"))
    {
      auto line = COMMAND_PARENT::options["set"].template as<std::string>();
      environment.append(std::string_view(line));
      if (environment.save() && COMMAND_PARENT::restart_service())
        return 0;
    }
    else
    {
      string contents;
      cerr << "+ Dumping environment file: " << environment.get_path() << endl;
      Crails::read_file(environment.get_path().string(), contents);
      cout << contents << endl;
    }
    return 0;
  }
};
