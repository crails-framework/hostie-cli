#pragma once
#include <string>
#include <string_view>
#include <optional>
#include <cstdint>
#include <crails/database_url.hpp>
#include <crails/cli/process.hpp>

struct MongoDatabase
{
  static constexpr std::string_view backup_key = "mongodb";

  std::string user, password;
  std::string database_name;
  std::string hostname = "127.0.0.1";
  unsigned short port = 27017;

  static const std::string password_charset;
  std::string auth_source = "admin";
  std::string root_user = "root";

  void from_url(const Crails::DatabaseUrl&);
  Crails::DatabaseUrl get_url() const;
  Crails::DatabaseUrl get_admin_url() const;

  bool user_exists() const;
  bool prepare_user() const;
  bool prepare_database() const;
  bool drop_database() const;

  bool run_query(const std::string_view query, const std::string_view database = "") const;
  bool collection_exists(const std::string_view) const;
  std::optional<std::uint64_t> disk_usage() const;

private:
  Crails::ExecutableCommand js_query_command(const std::string_view, const std::string_view database = "") const;
};
