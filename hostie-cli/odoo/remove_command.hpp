#pragma once
#include "live_instance_command.hpp"
#include "../databases/postgres.hpp"
#include "../user.hpp"
#include "../service.hpp"
#include "../nginx/remove_command.hpp"
#include <iostream>
#include <filesystem>

namespace Odoo
{
  class RemoveCommand : public Odoo::LiveInstanceCommand
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
      PostgresDatabase database;
      InstanceUser user;
      string database_url;

      service.app_name = options["name"].as<string>();
      user.name = environment.get_variable("APPLICATION_USER");
      database_url = environment.get_variable("DATABASE_URL");
      if (!service.running() || service.stop())
      {
        cerr << "loading database from url " << database_url << endl;
        database.from_url(string_view(database_url));
        cerr << "dropping database" << endl;
        database.drop_database();
        cerr << "removing service file" << endl;
        if (!filesystem::remove(service.service_file_path()))
          return -1;
        cerr << "removing environment file" << endl;
        if (!filesystem::remove(environment.get_path()))
          return -1;
        cerr << "removing var directory" << endl;
        if (!filesystem::remove_all(environment.get_variable("VAR_DIRECTORY")))
          return -1;
        cerr << "removing user" << endl;
        if (!user.delete_user())
          return -1;
        cerr << "removing nginx site" << endl;
        if (Nginx::remove_site(service.app_name))
          return -1;
        cerr << "wiping backups" << endl;
        wipe_backups();
        return 0;
      }
      else
        cerr << "failed to stop service" << endl;
      return -1;
    }
  };
}
