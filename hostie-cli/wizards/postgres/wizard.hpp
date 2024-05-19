#pragma once
#include <crails/utils/random_string.hpp>
#include <crails/cli/process.hpp>
#include <iostream>
#include "../../databases/postgres.hpp"

template<typename WIZARD_BASE>
class PostgresWizard : public WIZARD_BASE
{
  typedef WIZARD_BASE Super;
  HostieVariables store;
  std::string password;
public:
  bool is_installed() const { return store.has_variable("postgres_root"); }

  int run()
  {
    using namespace std;
    if (Super::install_requirements())
    {
      password = Crails::generate_random_string(PostgresDatabase::password_charset, 12);
      store.variable("postgres_root", password);
      if (alter_postgres_user())
      {
        store.save();
        return 0;
      }
      else
        cerr << "failed to alter postgres root user" << endl;
    }
    return -1;
  }

  bool alter_postgres_user()
  {
    using namespace std;
    stringstream command;

    command << "ALTER USER postgres WITH PASSWORD '" << password << "'";
    return run_sql_query(string_view(command.str()));
  }

  bool run_sql_query(const std::string_view query)
  {
    using namespace std;
    stringstream command;

    command << "su postgres -c \"psql -c " << quoted(query, '\'') << '"'; 
    return Crails::run_command(command.str());
  }
};
