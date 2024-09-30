#include <crails/cli/process.hpp>
#include <crails/utils/random_string.hpp>
#include <crails/utils/join.hpp>
#include <crails/utils/split.hpp>
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
#include "../hostie_variables.hpp"
#include "../php.hpp"

using namespace std;
using namespace Wordpress;

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
    database.password = Crails::generate_random_string(
      MysqlDatabase::password_charset, 32
    );

    if (!create_user(user) ||
        !prepare_runtime_directory(user) ||
        !prepare_wordpress(user))
      return cancel(user, database);

    environment.set_variables({
      {"APPLICATION_NAME",     options["name"].as<string>()},
      {"APPLICATION_USER",     user.name},
      {"APPLICATION_TYPE",     "Wordpress"},
      {"VAR_DIRECTORY",        var_directory.string()},
      {"DATABASE_URL",         database.get_url().to_string()}
    });

    if (generate_wp_config(user, database) &&
        generate_fpm_pool(user) &&
        prepare_environment_file() &&
        database.prepare_user() &&
        database.prepare_database())
    {
      state += DatabaseCreated;
      return 0;
    }
    return cancel(user, database);
  }
  return 13;
}

filesystem::path CreateCommand::find_wordpress_source() const
{
  return find_php_source("wordpress", "WORDPRESS_DIRECTORY");
}

bool CreateCommand::prepare_wordpress(const InstanceUser& user)
{
  filesystem::path wordpress_source = find_wordpress_source();
  filesystem::path wp_content_source = wordpress_source / "wp-content";
  filesystem::path wp_content_target = var_directory / "wp-content";
  filesystem::path upload_folder = wp_content_target / "uploads";

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
  filesystem::create_hard_link        (wp_content_source / "index.php", wp_content_target / "index.php");
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
  // Creating upload folder
  filesystem::create_directories(upload_folder);
  Crails::chown(upload_folder, user.name);
  Crails::chgrp(upload_folder, "www-data");
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
      << "define('FORCE_SSL_ADMIN', false);\n" // ssl is enforced through other means
      << "if (!defined('ABSPATH'))\n"
      << "  define('ABSPATH', " << quoted(var_directory.string(), '\'') << ");\n"
      << "if (!defined('WP_CONTENT_DIR'))\n"
      << "  define('WP_CONTENT_DIR', " << quoted((var_directory / "wp-content").string(), '\'') << ");\n"
      << "require_once(ABSPATH . 'wp-settings.php');\n";
    stream.close();
    Crails::chown(path, user.name);
    Crails::chgrp(path, HostieVariables::global->variable("web-group"));
    filesystem::permissions(path,
      filesystem::perms::owner_read | filesystem::perms::group_read |
      filesystem::perms::owner_exec | filesystem::perms::group_exec,
      filesystem::perm_options::replace);
    return true;
  }
  cerr << "failed to generate wp-config.php" << endl;
  return false;
}

int CreateCommand::cancel(InstanceUser& user, MysqlDatabase& database)
{
  if ((state & DatabaseCreated) > 0)
    database.drop_database();
  return PhpFpmCreator::cancel(user);
}
