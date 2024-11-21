#include <crails/utils/random_string.hpp>
#include <crails/cli/filesystem.hpp>
#include <crails/cli/process.hpp>
#include <crails/pbkdf2_hmac.hpp>
#include <crails/utils/base64.hpp>
#include <iostream>
#include <sstream>
#include "create.hpp"
#include "../service.hpp"
#include "../user.hpp"
#include "../databases/postgres.hpp"
#include "../hostie_variables.hpp"

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

void CreateCommand::options_description(boost::program_options::options_description& options) const
{
  StandardCreator::options_description(options);
  options.add_options()
    ("gevent-port,q", boost::program_options::value<unsigned short>(), "network port to use for long polling")
    ("without-demo", "disable demo data for modules");
}

unsigned short CreateCommand::gevent_port() const
{
  if (options.count("gevent-port"))
    return options["gevent-port"].as<unsigned short>();
  return options["port"].as<unsigned short>() + 1;
}

bool CreateCommand::generate_odoo_conf(const PostgresDatabase& database)
{
  stringstream stream;

  stream
    << "[options]\n"
    << "http_port = " << options["port"].as<unsigned short>() << '\n'
    << "gevent_port = " << gevent_port() << '\n'
    << "data_dir = " << var_directory.string() << '\n'
    << "logfile = " << (get_log_directory() / "server.log").string() << '\n'
    << "db_host = localhost\n"
    << "db_port = " << database.port << '\n'
    << "db_user = " << database.user << '\n'
    << "db_password = " << database.password << '\n'
    << "list_db = False\n"
    << "default_productivity_apps = True\n"
    << "without_demo = " << (options.count("without-demo") ? "True" : "False") << '\n'
    << "admin_passwd = " << encoded_admin_password << '\n'
    << "db_filter = ^" << database.database_name << "$\n";
  return Crails::write_file("odoo", odoo_rc_path().string(), stream.str());
}

string CreateCommand::start_command(const filesystem::path& bin, const PostgresDatabase& database) const
{
  stringstream stream;

  stream
    << bin
    << " -d " << quoted(database.database_name)
    << " -i " << quoted(database.database_name);
  return stream.str();
}

void CreateCommand::initialize_admin_password(const string& password)
{
  const string salt = Crails::generate_random_string("0123456789abcdefghijklmnopqrstuvwxyz", 22);
  const auto digest = Crails::Pbkdf2HmacDigest(password, salt, EVP_sha512());

  encoded_admin_password = digest.password_hash();
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
  {
    cout << "Located odoo at " << odoo_bin_path << endl;
    odoo_bin_path = filesystem::canonical(odoo_bin_path);
  }

  if (!create_user(user) || !prepare_runtime_directory(user))
    return cancel(user, database);

  database.user = user.name;
  database.database_name = options["name"].as<string>();
  database.password = Crails::generate_random_string(
    PostgresDatabase::password_charset, 32
  );

  service.app_name = options["name"].as<string>();
  service.start_command = start_command(odoo_bin_path, database);
  service.runtime_directory = var_directory;
  service.environment_path = environment.get_path();
  service.kill_mode = "mixed";

  environment.set_variables({
    {"APPLICATION_NAME", service.app_name},
    {"APPLICATION_USER", service.app_user},
    {"APPLICATION_TYPE", "Odoo"},
    {"APPLICATION_PORT", to_string(options["port"].as<unsigned short>())},
    {"GEVENT_PORT",      to_string(gevent_port())},
    {"VAR_DIRECTORY",    var_directory.string()},
    {"DATABASE_URL",     database.get_url().to_string()},
    {"ODOO_RC",          odoo_rc_path().string()}
  });

  initialize_admin_password(default_admin_password());

  if (generate_odoo_conf(database) &&
      prepare_environment_file() &&
      prepare_log_directory(service) && 
      service.require() &&
      prepare_database(database))
  {
    Crails::run_command({"chown", {service.app_user, var_directory.string()}});
    if (service.reload_service_files() && service.start())
    {
      unsigned int attempts = 0;
      cerr << "successfully started service " << service.app_name << endl;
      while (!database.table_exists("res_users") && attempts++ < 10)
        sleep(2);
      // for some unknown reason, the first attempt always fails:
      attempts = 0;
      while (!update_admin_password(database) && attempts++ < 10)
        sleep(2);
      if (attempts < 10)
      {
        cerr << "updated admin password" << endl;
        setup_base_url(database);
        return 0;
      }
      return 3;
    }
    else
      cerr << "failed to start service " << service.app_name << endl;
    return 2;
  }
  return cancel(user, database);
}

bool CreateCommand::update_admin_password(PostgresDatabase& database) const
{
  ostringstream query;

  query << "UPDATE res_users SET "
        << "login=" << quoted(default_admin_login(), '\'') << ", "
        << "password=" << quoted(encoded_admin_password, '\'') << ' '
        << "WHERE login='admin'";
  return database.run_query(query.str(), string_view(database.database_name));
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

bool CreateCommand::setup_base_url(PostgresDatabase& database) const
{
  bool success = options.count("domains");

  if (success)
  {
    const string url = *(options["domains"].as<vector<string>>().begin());
    ostringstream update_query, freeze_query;

    update_query
          << "UPDATE ir_config_parameter SET value=" << quoted(url, '\'')
          << " WHERE key='web.base.url'";
    freeze_query
          << "INSERT INTO ir_config_parameter(create_uid,write_uid,key,value)"
          << " VALUES(1,1,'web.base.url.freeze','True')";
    success = database.run_query(update_query.str(), string_view(database.database_name))
           && database.run_query(freeze_query.str(), string_view(database.database_name));
    if (!success)
      cerr << "Failed to set ir_config_parameter web.base.url" << endl;
  }
  else
    cerr << "No domains specified: ir_config_parameter web.base.url won't be initialized" << endl;
  return success;
}

int CreateCommand::cancel(InstanceUser& user, PostgresDatabase& database)
{
  if ((state & DatabaseCreated) > 0)
    database.drop_database();
  return StandardCreator::cancel(user);
}
