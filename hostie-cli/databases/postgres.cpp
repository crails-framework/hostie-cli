#include <crails/cli/process.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include "postgres.hpp"

using namespace std;

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

string PostgresDatabase::sql_query_command(const string_view query) const
{
  stringstream command, su_command;

  const string postgres_username = "postgres";
  const string postgres_password = "postgres";

  setenv("PGPASSWORD", postgres_password.c_str(), 1);
  cout << "sql query: " << query << endl;
  command << "psql"
          << " -U " << postgres_username
          << " -h " << hostname
          << " -p " << port
          << " -tAc " << quoted(query);
  cerr << "== " << command.str() << endl;
  return command.str();
  //su_command << "sudo -u postgres bash -c " << quoted(command.str());
  //return su_command.str(); 
}

bool PostgresDatabase::run_query(const string_view query) const
{
  return std::system(sql_query_command(query).c_str()) == 0;
}

bool PostgresDatabase::user_exists() const
{
  stringstream command;
  string query = "SELECT 1 FROM pg_roles WHERE rolname='" + user + '\'';

  command << sql_query_command(string_view(query))
          << " | grep 1";
  return std::system(command.str().c_str()) == 0;
}

bool PostgresDatabase::prepare_user() const
{
  string query;

  if (!user_exists())
    query = "CREATE USER " + user + " WITH PASSWORD '" + password + "';";
  else
    query = "ALTER USER " + user + " WITH PASSWORD '" + password + "';";
  return run_query(string_view(query));
}

bool PostgresDatabase::prepare_database() const
{
  string create_query;
  string grant_query;

  create_query = "CREATE DATABASE " + database_name + " WITH OWNER " + user;
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
  string query = "DROP DATABASE " + database_name;

  if (run_query(query))
    return true;
  else
    cerr << "failed to drop database " << database_name << endl;
  return false;
}
