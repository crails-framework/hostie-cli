#pragma once
#include "live_instance_command.hpp"

class RestoreCommand : public ::LiveInstanceCommand
{
public:
  std::string_view description() const override
  {
    return "restore a backup using crails-backup";
  }

  void options_description(boost::program_options::options_description& options) const override
  {
    using namespace std;
    InstanceCommand::options_description(options);
    options.add_options()
      ("id,i", boost::program_options::value<string>(), "id of the backup to restore");
  }

  int run() override
  {
    using namespace std;
    ostringstream command;
    string_view action = "restore";

    if (!options.count("id"))
      return -1;
    command
      << "/usr/local/bin/crails-backup " << action << ' '
      << "-n " << quoted(environment.get_project_name())
      << "--id " << options["id"].as<string>();
    cout << "+ " << command.str() << endl;
    return system(command.str().c_str()) == 0;
  }
};
