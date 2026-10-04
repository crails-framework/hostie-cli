#include <crails/cli/process.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include "mongodb.hpp"
#include "../hostie_variables.hpp"

using namespace std;

const string MongoDatabase::password_charset = "abcdefghijklmnopqrstuvwxyz"
                                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                               "0123456789-_";

static string js_string_literal(const string_view value)
{
  ostringstream stream;

  stream << quoted(value);
  return stream.str();
}

static uint64_t parse_trailing_integer(const string& output)
{
  size_t position = output.size();

  while (position > 0 && !isdigit(static_cast<unsigned char>(output[position - 1])))
    --position;
  if (position == 0)
    return 0;
  const size_t end = position;
  while (position > 0 && isdigit(static_cast<unsigned char>(output[position - 1])))
    --position;
  return stoull(output.substr(position, end - position));
}

void MongoDatabase::from_url(const Crails::DatabaseUrl& url)
{
  user = url.username;
  password = url.password;
  database_name = url.database_name;
  hostname = url.hostname;
  port = url.port;
}

Crails::DatabaseUrl MongoDatabase::get_url() const
{
  Crails::DatabaseUrl url;

  url.type = "mongodb";
  url.hostname = hostname;
  url.username = user;
  url.password = password;
  url.database_name = database_name;
  url.port = port;
  url.params = "authSource=" + auth_source;
  return url;
}

Crails::DatabaseUrl MongoDatabase::get_admin_url() const
{
  Crails::DatabaseUrl url;

  url.type = "mongodb";
  url.hostname = hostname;
  url.database_name = "admin";
  url.port = port;
  return url;
}

Crails::ExecutableCommand MongoDatabase::js_query_command(const string_view query, const string_view database) const
{
  Crails::ExecutableCommand command;
  const string root_password = HostieVariables::global->variable("mongodb_root");
  const string target_database = database.length() > 0 ? string(database) : "admin";

  setenv("MONGO_ROOT_PASSWORD", root_password.c_str(), 1);
  setenv("MONGO_USER_PASSWORD", password.c_str(), 1);
  command.path = "mongosh";
  command << "--quiet"
          << get_admin_url().to_string()
          << "--eval";
  command << string(
    "if (!db.auth(" + js_string_literal(root_user) + ", process.env.MONGO_ROOT_PASSWORD)) quit(1);\n"
    "const target = db.getSiblingDB(" + js_string_literal(target_database) + ");\n"
  ) + string(query);
  return command;
}

bool MongoDatabase::run_query(const string_view query, const string_view database) const
{
  return Crails::run_command(js_query_command(query, database));
}

bool MongoDatabase::user_exists() const
{
  string output;
  string script =
    "print(db.getUsers({users: [" + js_string_literal(user) + "]}).users.length ? 1 : 0)";

  if (Crails::run_command(js_query_command(string_view(script), string_view(database_name)), output))
    return output.find('1') != string::npos;
  return false;
}

bool MongoDatabase::prepare_user() const
{
  const string role = "{role: 'readWrite', db: " + js_string_literal(database_name) + "}";
  string script;

  if (!user_exists())
    script =
      "db.createUser({user: " + js_string_literal(user) + ", "
      "pwd: process.env.MONGO_USER_PASSWORD, "
      "roles: [" + role + "]})";
  else
    script =
      "db.updateUser(" + js_string_literal(user) + ", {roles: [" + role + "]}); "
      "db.changeUserPassword(" + js_string_literal(user) + ", process.env.MONGO_USER_PASSWORD)";
  return run_query(string_view(script));
}

bool MongoDatabase::prepare_database() const
{
  string script =
    "try { target.createCollection('init') } "
    "catch (e) { if (e.codeName != 'NamespaceExists') quit(1) }";

  if (run_query(string_view(script), string_view(database_name)))
    return true;
  cerr << "failed to prepare database " << database_name << endl;
  return false;
}

bool MongoDatabase::drop_database() const
{
  if (run_query(string_view("target.dropDatabase()"), string_view(database_name)))
    return true;
  cerr << "failed to drop database " << database_name << endl;
  return false;
}

bool MongoDatabase::collection_exists(const string_view name) const
{
  string output;
  string script =
    "print(target.getCollectionNames().includes(" + js_string_literal(name) + ") ? 1 : 0)";

  if (Crails::run_command(js_query_command(string_view(script), string_view(database_name)), output))
    return output.find('1') != string::npos;
  return false;
}

optional<uint64_t> MongoDatabase::disk_usage() const
{
  string output;
  string script = "print(Math.trunc(target.stats().storageSize))";

  if (Crails::run_command(js_query_command(string_view(script), string_view(database_name)), output))
    return parse_trailing_integer(output);
  cerr << "failed to measure the size of database " << database_name << endl;
  return {};
}
