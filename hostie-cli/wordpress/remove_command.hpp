#pragma once
#include "live_instance_command.hpp"
#include "../databases/mysql.hpp"
#include "../user.hpp"
#include "../nginx/remove_command.hpp"
#include <iostream>
#include <filesystem>

namespace Wordpress
{
  class RemoveCommand : public Wordpress::LiveInstanceCommand
  {
  public:
    std::string_view description() const override
    {
      return "permanently remove an instance";
    }

    int run() override
    {
      using namespace std;
      MysqlDatabase database;
      InstanceUser user;
      string database_url;

      user.name = environment.get_variable("APPLICATION_USER");
      database_url = environment.get_variable("DATABASE_URL");
      cerr << "loading database from url " << database_url << endl;
      database.from_url(string_view(database_url));
      cerr << "dropping database" << endl;
      database.drop_database();
      cerr << "removing environment file" << endl;
      if (!filesystem::remove(environment.get_path()))
        return -1;
      cerr << "removing var directory" << endl;
      if (!filesystem::remove_all(environment.get_variable("VAR_DIRECTORY")))
        return -1;
      cerr << "removing user" << endl;
      if (!user.delete_user())
        return -1;
      cerr << "removing php-fpm pool" << endl;
      if (!filesystem::remove(fpm_pool_path()))
        return -1;
      cerr << "removing nginx site" << endl;
      if (Nginx::remove_site(options["name"].as<string>()))
        return -1;
      cerr << "wiping backups" << endl;
      wipe_backups();
      return 0;
    }
  };
}
