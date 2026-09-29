#pragma once
#include "../live_instance_command.hpp"
#include <crails/cli/process.hpp>

namespace Html
{
  class DeployCommand : public LiveInstanceCommand
  {
  public:
    std::string_view application_type() const override
    {
      return "HTML";
    }

    std::string_view description() const override
    {
      return "deploys a tar archive as the HTML site";
    }
  
    void options_description(boost::program_options::options_description& options) const override
    {
      using namespace std;
      options.add_options()
        ("package,p", boost::program_options::value<string>(), "absolute path of the tarball to extract");
    }

    int run() override
    {
      using namespace std;
      string user      = environment.get_variable("APPLICATION_USER");
      string directory = environment.get_variable("VAR_DIRECTORY");
      string group     = HostieVariables::global->variable("web-group");
      string package   = options["package"].as<string>();

      if (filesystem::exists(package))
      {
        Crails::ExecutableCommand untar_command{"tar",   {}};
        Crails::ExecutableCommand chown_command{"chown", {"-R", user + ':' + group, directory}};
        Crails::ExecutableCommand chmod_file_command{"find", {directory, "-type", "f", "-exec", "chmod 640"}};
        Crails::ExecutableCommand chmod_dirs_command{"find", {directory, "-type", "d", "-exec", "chmod 750"}};

        cerr << "== " << untar_command << endl;
        if (!Crails::run_command(untar_command))
          return 2;
        cerr << "== " << chown_command << endl;
        if (!Crails::run_command(chown_command))
          return 2;
        cerr << "== " << chmod_file_command << endl;
        if (!Crails::run_command(chmod_file_command))
          return 2;
        cerr << "== " << chmod_dirs_command << endl;
        if (Crails::run_command(chmod_dirs_command))
          return 2;
        return 0;
      }
      else
        cerr << "file `" << package << "` does not exist" << endl;
      return 1;
    }
  };
}
