#include "usage.hpp"
#include "../databases/postgres.hpp"
#include "../databases/mysql.hpp"
#include "../databases/sql_helpers.hpp"
#include <crails/cli/process.hpp>
#include <crails/database_url.hpp>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sys/stat.h>

using namespace std;

typedef optional<uintmax_t> UsageBytes;

// st_blocks is expressed in units of 512 bytes (Linux and FreeBSD alike)
static uintmax_t blocks_usage(const struct stat& st)
{
  return static_cast<uintmax_t>(st.st_blocks) * 512;
}

static UsageBytes directory_usage(const filesystem::path& root)
{
  struct stat st;
  error_code error;
  uintmax_t total;

  if (lstat(root.c_str(), &st) == 0)
  {
    total = blocks_usage(st);
    filesystem::recursive_directory_iterator it(root, filesystem::directory_options::none, error), end;
    for (; !error && it != end ; it.increment(error))
    {
      if (lstat(it->path().c_str(), &st) != 0)
        continue ; // deleted while we were walking
      if (S_ISDIR(st.st_mode) || st.st_nlink <= 1)
        total += blocks_usage(st);
    }
  }
  else
  {
    cerr << root << ": " << strerror(errno) << endl;
    return {};
  }
  if (error)
  {
    cerr << root << ": " << error.message() << endl;
    return {};
  }
  return total;
}

static UsageBytes database_usage(const string& raw_url)
{
  try
  {
    Crails::DatabaseUrl url = string_view(raw_url);

    if (url.type == "postgres" || url.type == "postgresql")
    {
      PostgresDatabase database;
      database.from_url(url);
      return database.disk_usage();
    }
    else if (url.type == "mysql" || url.type == "mariadb")
    {
      MysqlDatabase database;
      database.from_url(url);
      return database.disk_usage();
    }
    cerr << "unsupported database type '" << url.type << '\'' << endl;
  }
  catch (const exception& error)
  {
    cerr << "invalid DATABASE_URL: " << error.what() << endl;
  }
  return nullopt;
}

static void write_bytes(ostream& out, const UsageBytes& value)
{
  if (value)
    out << *value;
  else
    out << "null";
}

int UsageCommand::run()
{
  if (!environment.already_exists())
  {
    cerr << "cannot find file " << environment.get_path() << endl;
    return 1;
  }
  environment.load();

  const string type = environment.get_variable("APPLICATION_TYPE");
  const string database_url = environment.get_variable("DATABASE_URL");
  const filesystem::path var_directory = environment.get_variable("VAR_DIRECTORY");
  const filesystem::path log_directory = filesystem::path("/var/log") / environment.get_project_name();
  error_code error;
  uintmax_t total = 0;
  bool complete = true;
  UsageBytes var_bytes, log_bytes, database_bytes;

  if (type.empty())
  {
    cerr << environment.get_path() << ": no APPLICATION_TYPE (unreadable file ?)" << endl;
    return 1;
  }

  if (!var_directory.empty())
    var_bytes = directory_usage(var_directory);
  else
    cerr << "missing VAR_DIRECTORY" << endl;
  const bool has_log_directory = filesystem::is_directory(log_directory, error);
  if (has_log_directory)
    log_bytes = directory_usage(log_directory);
  if (!database_url.empty())
    database_bytes = database_usage(database_url);

  // an absent log directory or database is fine, a failed measurement is not
  const struct { const UsageBytes& bytes; bool expected; } parts[] = {
    {var_bytes,      true},
    {log_bytes,      has_log_directory},
    {database_bytes, !database_url.empty()}
  };
  for (const auto& part : parts)
  {
    if (part.bytes)
      total += *part.bytes;
    else if (part.expected)
      complete = false;
  }

  cout << "{\n  \"var_directory\": ";
  write_bytes(cout, var_bytes);
  cout << ",\n  \"log_directory\": ";
  write_bytes(cout, log_bytes);
  cout << ",\n  \"database\": ";
  write_bytes(cout, database_bytes);
  cout << ",\n  \"total\": ";
  write_bytes(cout, complete ? UsageBytes(total) : nullopt);
  cout << "\n}\n";
  return 0;
}
