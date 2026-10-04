#include "database.hpp"
#include "postgres.hpp"
#include "mysql.hpp"
#include "mongodb.hpp"
#include <iostream>

using namespace std;

template<typename IMPL>
class DatabaseAdapter : public Database
{
protected:
  IMPL impl;
public:
  std::string_view type() const override { return IMPL::backup_key; }
  const std::string& password_charset() const override { return IMPL::password_charset; }

  void configure(const std::string& user, const std::string& database_name, const std::string& password) override
  {
    impl.user = user;
    impl.database_name = database_name;
    impl.password = password;
  }

  void from_url(const Crails::DatabaseUrl& url) override { impl.from_url(url); }
  Crails::DatabaseUrl get_url() const override { return impl.get_url(); }

  bool prepare_user() const override { return impl.prepare_user(); }
  bool prepare_database() const override { return impl.prepare_database(); }
  bool drop_database() const override { return impl.drop_database(); }
  std::optional<uint64_t> disk_usage() const override { return impl.disk_usage(); }
};

typedef DatabaseAdapter<PostgresDatabase> PostgresAdapter;
typedef DatabaseAdapter<MysqlDatabase>    MysqlAdapter;
typedef DatabaseAdapter<MongoDatabase>    MongodbAdapter;

unique_ptr<Database> make_database(string_view type)
{
  if (type == "postgres" || type == "postgresql")
    return make_unique<PostgresAdapter>();
  if (type == "mysql" || type == "mariadb")
    return make_unique<MysqlAdapter>();
  if (type == "mongodb")
    return make_unique<MongodbAdapter>();
  return nullptr;
}

unique_ptr<Database> make_database_from_url(string_view raw_url)
{
  try
  {
    Crails::DatabaseUrl url = raw_url;
    unique_ptr<Database> database = make_database(url.type);

    if (database)
      database->from_url(url);
    else
      cerr << "unsupported database type '" << url.type << '\'' << endl;
    return database;
  }
  catch (const exception& error)
  {
    cerr << "invalid DATABASE_URL: " << error.what() << endl;
  }
  return nullptr;
}
