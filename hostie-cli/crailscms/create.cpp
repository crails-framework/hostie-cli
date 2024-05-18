#include <crails/cli/process.hpp>
#include <crails/utils/random_string.hpp>
#include <crails/utils/join.hpp>
#include <crails/read_file.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include "create.hpp"
#include "../service.hpp"
#include "../user.hpp"
#include "../databases/postgres.hpp"
#include <iostream>

using namespace std;
using namespace CrailsCms;

extern filesystem::path crailscms_bin_dir;
  
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
    PostgresDatabase::password_charset, 12
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

bool CreateCommand::prepare_environment_file()
{
  if (options.count("env"))
  {
    for (const string& param : options["env"].as<vector<string>>())
    {
      string contents;

      if (param.find('=') != string::npos)
        contents = param;
      else if (!Crails::read_file(param, contents))
        cerr << "could not read environment file " << param << endl;
      environment.append(contents);
    }
  }
  return environment.save();
}

bool CreateCommand::prepare_runtime_directory(const InstanceUser& user)
{
  if (options.count("runtime-directory"))
    var_directory = filesystem::weakly_canonical(options["runtime-directory"].as<string>());
  else if (std::getenv("VAR_DIRECTORY") != 0)
    var_directory = filesystem::weakly_canonical(getenv("VAR_DIRECTORY")) / options["name"].as<string>();
  else
  {
    cerr << "could not deduce runtime directory" << endl;
    return false;
  }

  if (filesystem::is_directory(var_directory) || filesystem::create_directories(var_directory))
    cerr << "using runtime directory: " << var_directory << endl;
  else
  {
    cerr << "could not create runtime directory " << var_directory << endl;
    return false;
  }

  state += VarDirectoryCreated;

  stringstream chown_command, chgrp_command;
  chown_command << "chown -R " << user.name  << ' ' << var_directory;
  chgrp_command << "chown -R " << user.group << ' ' << var_directory;
  cout << "+ " << chown_command.str() << " && " << chgrp_command.str() << endl;
  return system(chown_command.str().c_str()) == 0 && system(chgrp_command.str().c_str()) == 0;
}

filesystem::path CreateCommand::get_log_directory() const
{
  return filesystem::path("/var/log") / environment.get_project_name();
}

bool CreateCommand::prepare_log_directory(const SystemService& service)
{
  filesystem::path directory = get_log_directory();

  if (filesystem::is_directory(directory) || filesystem::create_directories(directory))
  {
    string chown_command = "chown " + service.app_user + ' ' + directory.string();

    cerr << "using log directory: " << directory << endl;
    if (std::system(chown_command.c_str()) != 0)
    {
      cerr << "could not chown log directory " << directory << endl;
      return false;
    }
  }
  else
  {
    cerr << "could not create log directory " << directory << endl;
    return false;
  }
  state += LogDirectoryCreated;
  return true;
}

bool CreateCommand::create_user(InstanceUser& user)
{
  if (user.require())
  {
    state += UserCreated;
    return true;
  }
  return false;
}

int CreateCommand::cancel(InstanceUser& user, PostgresDatabase& database)
{
  filesystem::remove(environment.get_path());
  if ((state & VarDirectoryCreated) > 0)
    filesystem::remove_all(var_directory);
  if ((state & LogDirectoryCreated) > 0)
    filesystem::remove_all(get_log_directory());
  if ((state & DatabaseCreated) > 0)
    database.drop_database();
  if ((state & UserCreated) > 0)
    user.delete_user();
  return -1;
}
