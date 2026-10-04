#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <crails/database_url.hpp>

class Database
{
public:
  virtual ~Database() = default;

  // "postgres" or "mysql": also the key crails-backup uses for this engine
  virtual std::string_view type() const = 0;
  virtual const std::string& password_charset() const = 0;

  virtual void configure(const std::string& user, const std::string& database_name, const std::string& password) = 0;
  virtual void from_url(const Crails::DatabaseUrl&) = 0;
  virtual Crails::DatabaseUrl get_url() const = 0;

  virtual bool prepare_user() const = 0;
  virtual bool prepare_database() const = 0;
  virtual bool drop_database() const = 0;
  virtual std::optional<std::uint64_t> disk_usage() const = 0;
};

std::unique_ptr<Database> make_database(std::string_view type);
std::unique_ptr<Database> make_database_from_url(std::string_view url);
