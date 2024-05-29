#include <crails/cli/process.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include "postgres.hpp"
#include "../hostie_variables.hpp"

using namespace std;

const string PostgresDatabase::password_charset = "abcdefghijklmnopqrstuvwxyz"
                                                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                  "0123456789-_";

void PostgresDatabase::from_url(const Crails::DatabaseUrl& url)
{
  user = url.username;
  password = url.password;
  database_name = url.database_name;
  hostname = url.hostname;
  port = url.port;
}

Crails::DatabaseUrl PostgresDatabase::get_url() const
{
  Crails::DatabaseUrl url;

  url.type = "postgres";
  url.hostname = hostname;
  url.username = user;
  url.password = password;
  url.database_name = database_name;
  url.port = port;
  return url;
}

Crails::ExecutableCommand PostgresDatabase::sql_query_command(const string_view query, const string_view database) const
{
  Crails::ExecutableCommand command;
  const string postgres_username = "postgres";
  const string postgres_password = HostieVariables::global->variable("postgres_root");

  setenv("PGPASSWORD", postgres_password.c_str(), 1);
  cout << "sql query: " << query << endl;
  command.path = "psql";
  command 
          << "-U" << postgres_username
          << "-h" << hostname
          << "-p" << to_string(port);
  if (database.length() > 0)
    command << "-d" << database;
  command << "-tAc" << query;
  cerr << "== " << command << endl;
  return command;
}

bool PostgresDatabase::run_query(const string_view query, const string_view database) const
{
  return Crails::run_command(sql_query_command(query, database));
}

bool PostgresDatabase::table_exists(const string_view name) const
{
  stringstream query;

  query << "SELECT 1 FROM pg_tables WHERE tablename = '" << name << '\'';
  return run_query(query.str(), string_view(database_name));
}

bool PostgresDatabase::user_exists() const
{
  string output;
  string query = "SELECT 1 FROM pg_roles WHERE rolname='" + user + '\'';

  if (Crails::run_command(sql_query_command(string_view(query)), output))
    return output.find('1') != string::npos;
  return false;
}

bool PostgresDatabase::prepare_user() const
{
  string query;

  if (!user_exists())
    query = "CREATE USER \"" + user + "\" WITH PASSWORD '" + password + "';";
  else
    query = "ALTER USER \"" + user + "\" WITH PASSWORD '" + password + "';";
  return run_query(string_view(query));
}

bool PostgresDatabase::prepare_database() const
{
  string create_query;
  string grant_query;

  create_query = "CREATE DATABASE \"" + database_name + "\" WITH OWNER \"" + user + '"';
  if (run_query(create_query))
  {
    grant_query = "GRANT ALL PRIVILEGES ON DATABASE \"" + database_name + "\" TO \"" + user + '"';
    if (run_query(grant_query))
      return true;
    else
      cerr << "failed to grant user " << user << " permissions on database " << database_name << endl;
  }
  else
    cerr << "failed to create database " << database_name << endl;
  return false;
}

bool PostgresDatabase::drop_database() const
{
  string query = "DROP DATABASE \"" + database_name + '"';

  if (run_query(query))
    return true;
  else
    cerr << "failed to drop database " << database_name << endl;
  return false;
}
