#include <crails/utils/random_string.hpp>
#include <crails/cli/filesystem.hpp>
#include <crails/cli/process.hpp>
#include <iostream>
#include "create.hpp"
#include "../service.hpp"
#include "../user.hpp"
#include "../databases/postgres.hpp"

using namespace std;
using namespace Odoo;

static filesystem::path find_odoo_bin()
{
  const char* odoo_bin_path = getenv("ODOO_BIN_PATH");

  if (!odoo_bin_path)
  {
    string candidate = Crails::which("odoo");

    if (!candidate.length())
      candidate = Crails::which("odoo-bin");
    return candidate;
  }
  return filesystem::path(odoo_bin_path);
}

filesystem::path CreateCommand::odoo_rc_path() const
{
  return var_directory / "odoorc";
}

bool CreateCommand::generate_odoo_conf(const PostgresDatabase& database)
{
  stringstream stream;

  stream
    << "[options]\n"
    << "http_port = " << options["port"].as<unsigned short>() << '\n'
    << "data_dir = " << var_directory.string() << '\n'
    << "db_host = localhost\n"
    << "db_port = " << database.port << '\n'
    << "db_user = " << database.user << '\n'
    << "db_password = " << database.password << '\n'
    << "default_productivity_apps = True\n"
    << "admin_passwd = " << encoded_admin_password << '\n'
    << "db_filter = ^" << database.database_name << "$\n";
  return Crails::write_file("odoo", odoo_rc_path().string(), stream.str());
}

int CreateCommand::run()
{
  InstanceUser user;
  PostgresDatabase database;
  SystemService service;
  filesystem::path odoo_bin_path = find_odoo_bin();

  user.name = service.app_user = options["user"].as<string>();
  user.group = service.app_group = options["group"].as<string>();

  if (!filesystem::exists(odoo_bin_path))
  {
    cerr << "Could not locate odoo binary" << endl;
    return cancel(user, database);
  }
  else
    odoo_bin_path = filesystem::canonical(odoo_bin_path);

  if (!create_user(user) || !prepare_runtime_directory(user))
    return cancel(user, database);

  service.app_name = options["name"].as<string>();
  service.start_command = odoo_bin_path.string();
  service.kill_mode = "mixed";

  database.user = user.name;
  database.database_name = service.app_name;
  database.password = Crails::generate_random_string(
    PostgresDatabase::password_charset, 12
  );

  environment.set_variables({
    {"APPLICATION_NAME", service.app_name},
    {"APPLICATION_USER", service.app_user},
    {"APPLICATION_TYPE", "Odoo"},
    {"APPLICATION_PORT", to_string(options["port"].as<unsigned short>())},
    {"VAR_DIRECTORY",    var_directory.string()},
    {"DATABASE_URL",     database.get_url().to_string()},
    {"ODOO_RC",          odoo_rc_path().string()}
  });

  if (generate_odoo_conf(database) &&
      prepare_environment_file() &&
      prepare_log_directory(service) && 
      service.require() &&
      prepare_database(database))
  {
    if (service.reload_service_files() && service.start())
    {
      cerr << "successfully started service " << service.app_name << endl;
      return 0;
    }
    else
      cerr << "failed to start service " << service.app_name << endl;
    return 2;
  }
  return -1;
}

bool CreateCommand::prepare_database(const PostgresDatabase& database)
{
  if (database.prepare_user() && database.prepare_database())
  {
    state += DatabaseCreated;
    return true;
  }
  return false;
}

int CreateCommand::cancel(InstanceUser& user, PostgresDatabase& database)
{
  if ((state & DatabaseCreated) > 0)
    database.drop_database();
  return StandardCreator::cancel(user);
}
