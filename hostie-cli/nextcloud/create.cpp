#include <crails/utils/random_string.hpp>
#include <crails/utils/split.hpp>
#include <crails/utils/semantics.hpp>
#include <crails/cli/process.hpp>
#include <crails/cli/with_path.hpp>
#include <sstream>
#include <fstream>
#include "create.hpp"
#include "../user.hpp"
#include "../databases/mysql.hpp"
#include "../file_ownership.hpp"
#include "../hostie_variables.hpp"

using namespace std;
using namespace NextCloud;

int CreateCommand::run()
{
  if (InstanceCommand::run() == 0)
  {
    InstanceUser user;
    MysqlDatabase database;

    user.name = options["user"].as<string>();
    user.group = HostieVariables::global->variable("web-group");
    database.user = user.name;
    database.database_name = options["name"].as<string>();
    database.database_options = "CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci";
    database.password = Crails::generate_random_string(
      MysqlDatabase::password_charset, 32
    );

    if (!create_user(user) ||
        !prepare_runtime_directory(user) ||
        !prepare_nextcloud(user))
      return cancel(user, database);

    environment.set_variables({
      {"APPLICATION_NAME",     options["name"].as<string>()},
      {"APPLICATION_USER",     user.name},
      {"APPLICATION_TYPE",     "NextCloud"},
      {"VAR_DIRECTORY",        var_directory.string()},
      {"DATABASE_URL",         database.get_url().to_string()}
    });

    if (generate_fpm_pool(user) &&
        prepare_environment_file() &&
        database.prepare_user() &&
        database.prepare_database())
    {
      state += DatabaseCreated;
      if (install_nextcloud(user, database) && post_install_nextcloud(user))
        return 0;
      else
        cerr << "Failed to run `occ maintenance:install` and post-install hooks" << endl;
      return 14;
    }
    return cancel(user, database);
  }
  return 13;
}

filesystem::path CreateCommand::find_nextcloud_source() const
{
  return find_php_source("nextcloud", "NEXTCLOUD_DIRECTORY");
}

string CreateCommand::find_nextcloud_version() const
{
  filesystem::path path = find_nextcloud_source();
  Crails::WithPath path_lock(path);
  string output;

  if (Crails::run_command("php version-printer.php", output))
    cout << "NextCloud version is: " << output << endl;
  else
    cerr << "Could not determine NextCloud version !" << endl;
  return Crails::strip(output, '\n');
}

bool CreateCommand::prepare_nextcloud(const InstanceUser& user)
{
  filesystem::path nextcloud_source = find_nextcloud_source();
  filesystem::path config_dir = var_directory / "config";
  filesystem::path core_dir = var_directory / "core";
  filesystem::path dist_dir = var_directory / "dist";
  filesystem::path thirdparty_dir = var_directory / "3rdparty";

  if (nextcloud_source.empty())
  {
    cerr << "nextcloud source not found" << endl;
    return false;
  }
  try
  {
    for (const auto& entry : filesystem::directory_iterator(nextcloud_source))
    {
      filesystem::path path = entry.path();

      if (path.filename().string() == "core")
        filesystem::create_directory_symlink(path, core_dir);
      else if (path.filename().string() == "dist")
        filesystem::create_directory_symlink(path, dist_dir);
      else if (path.filename().string() == "3rdparty")
        filesystem::create_directory_symlink(path, thirdparty_dir);
      else
        filesystem::copy(path, var_directory / path.filename().string(), filesystem::copy_options::overwrite_existing | filesystem::copy_options::recursive);
    }
    for (const auto& entry : filesystem::recursive_directory_iterator(var_directory))
    {
      filesystem::path path = entry.path();
      filesystem::perms perms;

      if (path.string().find(core_dir.string()) != string::npos ||
          path.string().find(dist_dir.string()) != string::npos ||
          path.string().find(thirdparty_dir.string()) != string::npos)
        continue ;
      if (filesystem::is_directory(path) || path.extension().string() == ".php")
      {
        perms = filesystem::perms::owner_all |
                filesystem::perms::group_read | filesystem::perms::group_exec;
      }
      else
      {
        perms = filesystem::perms::owner_read | filesystem::perms::owner_write |
                filesystem::perms::group_read;
      }
      filesystem::permissions(path, perms);
      Crails::chown(path, user.name);
      Crails::chgrp(path, user.group);
    }
  }
  catch (const std::exception& err)
  {
    cerr << "Failed to prepare nextcloud: " << err.what() << endl;
  }
  return true;
}

bool CreateCommand::run_occ_command(const InstanceUser& user, const std::string& command)
{
  stringstream stream;
  Crails::WithPath path_lock(var_directory);

  stream
    << "sudo -u " << user.name << ' '
    << "php occ " << command;
  cout << "+ " << stream.str() << endl;
  if (options.count("skip-install") > 0)
    return true;
  return std::system(stream.str().c_str()) == 0;
}

bool CreateCommand::install_nextcloud(const InstanceUser& user, const MysqlDatabase& database)
{
  stringstream command;

  command
    << "maintenance:install --no-interaction"
    << " --database mysql"
    << " --database-host " << quoted(database.hostname, '\'')
    << " --database-port " << database.port
    << " --database-user " << quoted(database.user, '\'')
    << " --database-pass " << quoted(database.password, '\'')
    << " --database-name " << quoted(database.database_name, '\'')
    << " --admin-user " << quoted(default_admin_login(), '\'')
    << " --admin-pass " << quoted(default_admin_password(), '\'');
  return run_occ_command(user, command.str());
}

bool CreateCommand::post_install_nextcloud(const InstanceUser& user)
{
  auto domains = Crails::split(environment.get_variable("HOSTIE_DOMAINS"), ';');
  bool success = true;

  if (domains.size())
    success = configure_overwrite_cli_url(user, *domains.begin()) && configure_trusted_domains(user, domains);
  return success
    && run_occ_command(user, "config:system:set maintenance_window_start --type=integer --value=2");
}

bool CreateCommand::configure_overwrite_cli_url(const InstanceUser& user, const std::string& domain_name)
{
  stringstream command;

  command
    << "config:system:set "
    << "overwrite.cli.url --value=" << quoted("https://" + domain_name);
  return run_occ_command(user, command.str());
}

bool CreateCommand::configure_trusted_domains(const InstanceUser& user, const list<string>& domains)
{
  int i = 0;

  run_occ_command(user, "config:system:delete trusted_domains");
  for (const string& domain_name : domains)
  {
    stringstream command;

    command
      << "config:system:set "
      << "trusted_domains " << i++ << " --value=" << quoted(domain_name);
    run_occ_command(user, command.str());
  }
  return true;
}

void CreateCommand::append_custom_fpm_pool_settings(ostream& stream)
{
  stream
    << "php_value[memory_limit] = 512M\n"
    << "env[HOSTNAME] = $HOSTNAME\n"
    << "env[PATH] = /usr/local/bin:/usr/bin:/bin\n"
    << "env[TMP] = /tmp\n"
    << "env[TMPDIR] = /tmp\n"
    << "env[TEMP] = /tmp\n";
}

int CreateCommand::cancel(InstanceUser& user, MysqlDatabase& database)
{
  if ((state & DatabaseCreated) > 0)
    database.drop_database();
  return PhpFpmCreator::cancel(user);
}
