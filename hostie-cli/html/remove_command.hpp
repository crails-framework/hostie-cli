#pragma once

namespace Html
{
  class RemoveCommand : public LiveInstanceCommand
  {
  public:
    std::string_view application_type() const override
    {
      return "HTML";
    }

    std::string_view description() const override
    {
      return "permanently remove an instance";
    }

    int run() override
    {
      using namespace std;
      InstanceUser user;

      user.name = environment.get_variable("APPLICATION_USER");
      cerr << "removing nginx site" << endl;
      Nginx::remove_site(options["name"].as<string>());
      cerr << "removing environment file" << endl;
      if (!filesystem::remove(environment.get_path()))
        return -1;
      cerr << "removing var directory" << endl;
      if (!filesystem::remove_all(environment.get_variable("VAR_DIRECTORY")))
        return -1;
      cerr << "removing user" << endl;
      if (!user.delete_user())
        return -1;
      cerr << "wiping backups" << endl;
      wipe_backups();
      return 0;
    }
  };
}
