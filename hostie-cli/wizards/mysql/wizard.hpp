#pragma once
#include <crails/utils/random_string.hpp>
#include <crails/cli/process.hpp>
#include <iostream>
#include "../../databases/mysql.hpp"

template<typename WIZARD_BASE>
class MysqlWizard : public WIZARD_BASE
{
  typedef WIZARD_BASE Super;
  HostieVariables store;
  std::string password;
public:
  bool is_installed() const { return store.has_variable("mysql_root"); }

  int run()
  {
    using namespace std;
    if (Super::install_requirements())
    {
      password = Crails::generate_random_string(MysqlDatabase::password_charset, 12);
      store.variable("mysql_root", password);
      if (create_mysql_user() && grant_privileges())
      {
        store.save();
        return 0;
      }
      else
        cerr << "failed to create mysql root user" << endl;
    }
    return -1;
  }

  bool create_mysql_user()
  {
    using namespace std;
    stringstream command;

    command << "CREATE USER 'root'@'localhost' IDENTIFIED BY '" << password << "';";
    return run_sql_query(string_view(command.str()));
  }

  bool grant_privileges()
  {
    return run_sql_query("GRANT ALL PRIVILEGES ON *.* TO 'root'@'localhost' WITH GRAND OPTION;");
  }

  bool run_sql_query(const std::string_view query)
  {
    using namespace std;
    stringstream command;

    command << "mysql -e " << quoted(query);
    return Crails::run_command(command.str());
  }
};
