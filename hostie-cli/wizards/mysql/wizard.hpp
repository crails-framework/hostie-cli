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
  bool mysql_password_required = false;
protected:
  std::string service_name;
public:
  bool is_installed() const { return store.has_variable("mysql_root"); }

  int run()
  {
    using namespace std;
    if (Super::install_requirements())
    {
      password = Crails::generate_random_string(MysqlDatabase::password_charset, 12);
      store.variable("mysql_root", password);
      if (start_service() && create_mysql_user() && grant_privileges() && enable_service())
      {
        store.save();
        return 0;
      }
      else
        cerr << "failed to create mysql root user" << endl;
    }
    return -1;
  }

  virtual bool start_service() = 0;
  virtual bool enable_service() = 0;

  bool create_mysql_user()
  {
    using namespace std;
    ostringstream command;

    command << "CREATE USER 'root'@'localhost' IDENTIFIED BY '" << password << "';";
    if (!run_sql_query(string_view(command.str())))
    {
      ostringstream alter_command;
      alter_command << "ALTER USER 'root'@'localhost' IDENTIFIED BY '" << password << "';";
      return run_sql_query(string_view(alter_command.str()));
    }
    return true;
  }

  bool grant_privileges()
  {
    mysql_password_required = true;
    return run_sql_query("GRANT ALL PRIVILEGES ON *.* TO 'root'@'localhost' WITH GRANT OPTION;");
  }

  bool run_sql_query(const std::string_view query)
  {
    using namespace std;
    ostringstream command;

    if (mysql_password_required)
      command << "MYSQL_PWD=" << quoted(password) << ' ';
    command << "mysql -u root -e " << quoted(query);
    return std::system(command.str().c_str()) == 0;
  }
};
