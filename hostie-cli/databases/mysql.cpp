#include <crails/cli/process.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include "mysql.hpp"
#include "../hostie_variables.hpp"

using namespace std;

const string MysqlDatabase::password_charset = "abcdefghijklmnopqrstuvwxyz"
                                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                               "0123456789-_";

void MysqlDatabase::from_url(const Crails::DatabaseUrl& url)
{
  user = url.username;
  password = url.password;
  database_name = url.database_name;
  hostname = url.hostname;
  port = url.port;
}

Crails::DatabaseUrl MysqlDatabase::get_url() const
{
  Crails::DatabaseUrl url;

  url.type = "mysql";
  url.hostname = hostname;
  url.username = user;
  url.password = password;
  url.database_name = database_name;
  url.port = port;
  return url;
}

string MysqlDatabase::sql_query_command(const string_view query) const
{
  stringstream command, su_command;

  const string mysql_username = "root";
  const string mysql_password = HostieVariables::global->variable("mysql_root");

  setenv("MYSQL_PWD", mysql_password.c_str(), 1);
  cout << "sql query: " << query << endl;
  command << "mysql"
          << " -u " << mysql_username
        //<< " -h " << hostname
          << " -P " << port
          << " -e " << quoted(query);
  cerr << "== " << command.str() << endl;
  return command.str();
}

bool MysqlDatabase::run_query(const string_view query) const
{
  return std::system(sql_query_command(query).c_str()) == 0;
}

bool MysqlDatabase::user_exists() const
{
  stringstream command;
  string query = "SELECT COUNT(User) FROM mysql.user WHERE User='" + user + '\'';

  command << sql_query_command(string_view(query))
          << " | grep 1";
  return std::system(command.str().c_str()) == 0;
}

bool MysqlDatabase::prepare_user() const
{
  string query;

  if (!user_exists())
    query = "CREATE USER '" + user + "'@'localhost' IDENTIFIED BY '" + password + "';";
  else
    query = "ALTER USER '" + user + "'@'localhost' IDENTIFIED BY '" + password + "';";
  return run_query(string_view(query));
}

bool MysqlDatabase::prepare_database() const
{
  string create_query;
  string grant_query;

  create_query = "CREATE DATABASE " + database_name + ";";
  if (run_query(create_query))
  {
    grant_query = "GRANT ALL PRIVILEGES ON " + database_name + ".* TO '" + user + "'@'localhost';";
    if (run_query(grant_query))
      return true;
    else
      cerr << "failed to grant user " << user << " permissions on database " << database_name << endl;
  }
  else
    cerr << "failed to create database " << database_name << endl;
  return false;
}

bool MysqlDatabase::drop_database() const
{
  string query = "DROP DATABASE " + database_name + ';';

  if (run_query(query))
    return true;
  else
    cerr << "failed to drop database " << database_name << endl;
  return false;
}
