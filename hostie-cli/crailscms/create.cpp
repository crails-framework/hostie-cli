#include <crails/cli/process.hpp>
#include <crails/utils/random_string.hpp>
#include <crails/utils/join.hpp>
#include <crails/read_file.hpp>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include "create.hpp"
#include "../service.hpp"
#include "../user.hpp"
#include "../databases/postgres.hpp"
#include "../hostie_variables.hpp"

using namespace std;
using namespace CrailsCms;

bool CreateCommand::initialize(int argc, const char** argv)
{
  if (HostieVariables::global->has_variable("crailscms-bin-path"))
  {
    crailscms_bin_dir = HostieVariables::global->variable("crailscms-bin-path");
    return StandardCreator::initialize(argc, argv);
  }
  else
    cerr << "CrailsCMS not installed: missing crailscms-bin-path property from rc file." << endl;
  return false;
}
  
int CreateCommand::run()
{
  const char* share_path = getenv("SHARE_PATH");
  InstanceUser user;
  PostgresDatabase database;
  SystemService service;

  if (!share_path)
  {
    cerr << "undefined SHARE_PATH environment variable" << endl;
    return -1;
  }

  user.name = service.app_user = options["user"].as<string>();
  user.group = service.app_group = options["group"].as<string>();

  if (!create_user(user) || !prepare_runtime_directory(user))
    return cancel(user, database);

  service.app_name = options["name"].as<string>();
  service.start_command = (crailscms_bin_dir / "start.sh").string();
  service.stop_command = (crailscms_bin_dir / "stop.sh").string();
  service.runtime_directory = var_directory;
  service.environment_path = environment.get_path();
  database.user = user.name;
  database.database_name = service.app_name;
  database.password = Crails::generate_random_string(
    PostgresDatabase::password_charset, 32
  );

  vector<string> link_paths{
    "$LD_LIBRARY_PATH",
    "$CRAILS_CMS_PLUGIN_PATH",
    "/usr/local/lib",
    "/usr/local/lib/libcrails-cms",
    crailscms_bin_dir.string()
  };

  environment.set_variables({
    {"APPLICATION_NAME",       service.app_name},
    {"APPLICATION_USER",       service.app_user},
    {"APPLICATION_TYPE",       "CrailsCMS"},
    {"APPLICATION_HOSTNAME",   "127.0.0.1"}, // only reachable to the local nginx instance
    {"APPLICATION_HOST",       "127.0.0.1"}, // TODO remove this potentially unused variable ?
    {"APPLICATION_PORT",       to_string(options["port"].as<unsigned short>())},
    {"CRAILS_CMS_PLUGIN_PATH", ""},
    {"LD_LIBRARY_PATH",        Crails::join(link_paths, ':')},
    {"VAR_DIRECTORY",          var_directory.string()},
    {"SHARE_PATH",             share_path},
    {"DATABASE_URL",           database.get_url().to_string()}
  });

  if (prepare_environment_file() &&
      prepare_log_directory(service) &&
      service.require() &&
      prepare_database(service, database))
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
  return cancel(user, database);
}

bool CreateCommand::prepare_database(const SystemService& service, const PostgresDatabase& database)
{
  if (database.prepare_user() && database.prepare_database())
  {
    state += DatabaseCreated;
    return migrate_database(service);
  }
  return false;
}

bool CreateCommand::migrate_database(const SystemService& service)
{
  stringstream command, migrate_command;
  filesystem::path migrate_task =
    crailscms_bin_dir / "tasks" / "odb_migrate" / "task";

  migrate_command << migrate_task << " odb";
  if (run_command_as_app_user(migrate_command.str()) != 0)
  {
    cerr << "failed to migrate database" << endl;
    return false;
  }
  return true;
}

int CreateCommand::cancel(InstanceUser& user, PostgresDatabase& database)
{
  if ((state & DatabaseCreated) > 0)
    database.drop_database();
  return StandardCreator::cancel(user);
}
