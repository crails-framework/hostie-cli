#pragma once
#include <string>
#include <string_view>
#include <crails/database_url.hpp>

struct PostgresDatabase
{
  std::string user, password;
  std::string database_name;
  std::string hostname = "127.0.0.1";
  unsigned short port = 5432;

  void from_url(const Crails::DatabaseUrl&);
  Crails::DatabaseUrl get_url() const;

  bool user_exists() const;
  bool prepare_user() const;
  bool prepare_database() const;
  bool drop_database() const;

  bool run_query(const std::string_view) const;

private:
  std::string sql_query_command(const std::string_view) const;
};
