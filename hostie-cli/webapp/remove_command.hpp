#pragma once
#include "live_instance_command.hpp"
#include "deployment.hpp"
#include "../databases/database.hpp"
#include "../user.hpp"
#include "../service.hpp"
#include "../nginx/remove_command.hpp"
#include <iostream>
#include <filesystem>

namespace WebApp
{
  class RemoveCommand : public WebApp::LiveInstanceCommand
  {
  public:
    std::string_view description() const override
    {
      return "permanently remove an instance";
    }

    int run() override
    {
      using namespace std;
      SystemService service;
      InstanceUser user;
      Deployment deployment(options["name"].as<string>());
      const string database_url = environment.get_variable("DATABASE_URL");
      const filesystem::path var_directory = environment.get_variable("VAR_DIRECTORY");
      error_code error;

      service.app_name = deployment.name;
      user.name = environment.get_variable("APPLICATION_USER");
      // running() is simply false for an instance that was never deployed
      if (service.running() && !service.stop())
      {
        cerr << "failed to stop service" << endl;
        return -1;
      }
      cerr << "removing nginx site" << endl;
      Nginx::remove_site(service.app_name);
      if (!database_url.empty())
      {
        cerr << "dropping database" << endl;
        if (auto database = make_database_from_url(database_url))
          database->drop_database();
        else
          cerr << "could not load the database from DATABASE_URL, skipping" << endl;
      }
      cerr << "removing what crails-deploy installed (service, release environment)" << endl;
      deployment.remove_artifacts();
      cerr << "removing environment file" << endl;
      if (!filesystem::remove(environment.get_path(), error))
        return -1;
      if (!var_directory.empty())
      {
        cerr << "removing var directory" << endl;
        filesystem::remove_all(var_directory, error);
        if (error)
          return -1;
      }
      cerr << "removing user" << endl;
      if (!user.delete_user())
        return -1;
      cerr << "wiping backups" << endl;
      wipe_backups();
      cerr << "note: the application files installed by crails-deploy "
           << "(<root>/bin/" << deployment.name << ", <root>/share/" << deployment.name
           << "; root defaults to /usr/local) were left in place" << endl;
      return 0;
    }
  };
}
