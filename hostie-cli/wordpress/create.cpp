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
#include "../user.hpp"
#include "../databases/mysql.hpp"
#include "../file_ownership.hpp"
#include "php.hpp"

using namespace std;
using namespace Wordpress;

extern filesystem::path crailscms_bin_dir;
 
int CreateCommand::run()
{
  if (InstanceCommand::run() == 0)
  {
    InstanceUser user;
    MysqlDatabase database;

    user.name = options["user"].as<string>();
    user.group = options["group"].as<string>();
    database.user = user.name;
    database.database_name = options["name"].as<string>();
    database.password = Crails::generate_random_string("abcdefghijklmnopqrstuvwxyz"
                                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                       "0123456789-_", 12);

    if (!prepare_runtime_directory(user))
      return cancel(user, database);

    environment.set_variables({
      {"APPLICATION_NAME",     options["name"].as<string>()},
      {"APPLICATION_USER",     user.name},
      {"APPLICATION_TYPE",     "Wordpress"},
      {"VAR_DIRECTORY",        var_directory.string()},
      {"DATABASE_URL",         database.get_url().to_string()}
    });

    if (environment.save() &&
        generate_wp_config(user, database) &&
        generate_fpm_pool(user) &&
        create_user(user) &&
        database.prepare_user() &&
        database.prepare_database())
    {
      state += DatabaseCreated;
/*
      ostringstream perm_cmd;
      perm_cmd << "chown -R " << user.name << ' ' << var_directory;
      perm_cmd << " && ";
      perm_cmd << "chgrp -R www-data " << var_directory;
      perm_cmd << " && ";
      perm_cmd << "chmod g+w " << (var_directory / "wp-config.php");
      if (std::system(perm_cmd.str().c_str()) != 0)
        cerr << "failed to set permission for php-fpm in " << var_directory << endl;
*/
      return 0;
    }
    return cancel(user, database);
  }
  return 13;
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

bool CreateCommand::prepare_runtime_directory(const InstanceUser& user)
{
  if (options.count("runtime-directory"))
    var_directory = filesystem::weakly_canonical(options["runtime-directory"].as<string>());
  else if (std::getenv("SITES_DIRECTORY") != 0)
    var_directory = filesystem::weakly_canonical(getenv("SITES_DIRECTORY")) / options["name"].as<string>();
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
  return prepare_wordpress(user);
}

filesystem::path CreateCommand::find_wordpress_source() const
{
  if (options.count("wordpress-source"))
    return filesystem::weakly_canonical(options["wordpress-source"].as<string>());
  else if (std::getenv("WORDPRESS_DIRECTORY") != 0)
    return filesystem::weakly_canonical(getenv("WORDPRESS_DIRECTORY"));
  cerr << "could not deduce wordpress directory" << endl;
  return filesystem::path();
}

bool CreateCommand::prepare_wordpress(const InstanceUser& user)
{
  filesystem::path wordpress_source = find_wordpress_source();
  filesystem::path wp_content_source = wordpress_source / "wp-content";
  filesystem::path wp_content_target = var_directory / "wp-content";

  if (wordpress_source.empty())
  {
    cerr << "wordpress source not found" << endl;
    return false;
  }
  // Linking root directories
  for (const auto& entry : filesystem::directory_iterator(wordpress_source))
  {
    filesystem::path path = entry.path();

    if (filesystem::is_directory(path))
    {
      if (filesystem::equivalent(path, wp_content_source)) continue;
      filesystem::create_directory_symlink(path, var_directory / path.filename());
    }
    else if (path.filename() != "wp-config.php")
      filesystem::create_hard_link(path, var_directory / path.filename());
  }
  // Creating wp-content
  filesystem::create_directories(wp_content_target);
  filesystem::create_hard_link         (wp_content_source / "index.php", wp_content_target / "index.php");
  filesystem::create_directory_symlink(wp_content_source / "languages", wp_content_target / "languages");
  for (const string& name : vector<string>{"plugins", "themes"})
  {
    filesystem::path source_folder = wp_content_source / name;
    filesystem::path target_folder = wp_content_target / name;

    filesystem::create_directories(target_folder);
    Crails::chown(target_folder, user.name);
    Crails::chgrp(target_folder, "www-data");
    for (const auto& entry : filesystem::directory_iterator(source_folder))
    {
      filesystem::path path = entry.path();

      if (filesystem::is_directory(path))
        filesystem::create_directory_symlink(path, target_folder / path.filename());
      else
        filesystem::create_hard_link(path, target_folder / path.filename());
    }
  }
  return true;
}

bool CreateCommand::generate_wp_config(const InstanceUser& user, const MysqlDatabase& database)
{
  filesystem::path path = var_directory / "wp-config.php";
  ofstream stream(path);
  Crails::DatabaseUrl database_url = database.get_url();
  vector<string> secret_keys{
    "AUTH_KEY", "SECURE_AUTH_KEY", "LOGGED_IN_KEY", "NONCE_KEY", "AUTH_SALT",
    "SECURE_AUTH_SALT", "LOGGED_IN_SALT", "NONCE_SALT"
  };

  if (stream.is_open())
  {
    stream
      << "<?php\n"
      << "define('DB_NAME', " << quoted(database_url.database_name, '\'') << ");\n"
      << "define('DB_USER', " << quoted(database_url.username, '\'') << ");\n"
      << "define('DB_PASSWORD', " << quoted(database_url.password, '\'') << ");\n"
      << "define('DB_HOST', " << quoted(database.hostname, '\'') << ");\n";
    stream
      << "define('DB_CHARSET', 'utf8');\n"
      << "define('DB_COLLATE', '');\n";
    for (const auto& key : secret_keys)
    {
      stream
        << "define('" << key << "', " << quoted(Crails::generate_random_string(32), '\'') << ");\n";
    }
    stream
      << "$table_prefix = 'wp_';\n"
      << "define('WP_DEBUG', false);\n"
      << "if (!defined('ABSPATH'))\n"
      << "  define('ABSPATH', " << quoted(var_directory.string(), '\'') << ");\n"
      << "if (!defined('WP_CONTENT_DIR'))\n"
      << "  define('WP_CONTENT_DIR', " << quoted((var_directory / "wp-content").string(), '\'') << ");\n"
      << "require_once(ABSPATH . 'wp-settings.php');\n";
    stream.close();
    Crails::chown(path, user.name);
    Crails::chgrp(path, "www-data");
    filesystem::permissions(path,
      filesystem::perms::owner_read | filesystem::perms::group_read |
      filesystem::perms::owner_exec | filesystem::perms::group_exec,
      filesystem::perm_options::replace);
    return true;
  }
  cerr << "failed to generate wp-config.php" << endl;
  return false;
}

bool CreateCommand::generate_fpm_pool(const InstanceUser& user)
{
  ofstream stream(fpm_pool_path(environment));

  if (stream.is_open())
  {
    stream
      << '[' << environment.get_project_name() << ']' << '\n'
      << "user = " << user.name << '\n'
      << "group = www-data\n"
      << "listen = /run/php/php" << php_version() << "-fpm.sock\n";
    stream.close();
    state += FpmPoolCreated;
    return true;
  }
  else
    cerr << "failed to create php fpm pool at " << fpm_pool_path(environment) << endl;
  return false;
}

int CreateCommand::cancel(InstanceUser& user, MysqlDatabase& database)
{
  filesystem::remove(environment.get_path());
  if ((state & VarDirectoryCreated) > 0)
    filesystem::remove_all(var_directory);
  if ((state & DatabaseCreated) > 0)
    database.drop_database();
  if ((state & UserCreated) > 0)
    user.delete_user();
  if ((state & FpmPoolCreated) > 0)
    filesystem::remove(fpm_pool_path(environment));
  return -1;
}
