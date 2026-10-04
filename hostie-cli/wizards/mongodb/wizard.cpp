#include "wizard.hpp"
#include <crails/utils/random_string.hpp>
#include <crails/cli/process.hpp>
#include <crails/read_file.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;
using namespace MongoDB;

int Wizard::run()
{
  password = Crails::generate_random_string("ABCDEFGHIJKLMNOPQRSTWXYZ-_abcdefghijklmnopqrstuvwxyz0123456789", 12);
  store.variable("mongodb", "1");
  store.variable("mongodb_root", password);
  store.save();
  if (prepare_root_user())
  {
    if (enable_authorization())
    {
      if (authenticate_root())
        return 0;
      else
        cerr << "mongodb installed successfully, but authentication test failed" << endl;
    }
    else
      cerr << "mongodb wizard failed to enable authorization" << endl;
  }
  else
    cerr << "mongodb wizard failed to prepare user" << endl;
  return -1;
}

bool Wizard::prepare_root_user() const
{
  setenv("MONGO_ROOT_PASSWORD", password.c_str(), 1);
  std::string script =
    "try {"
    "  db.getSiblingDB('admin').createUser({"
    "    user: 'root', pwd: process.env.MONGO_ROOT_PASSWORD, "
    "    roles: ['root']"
    "  })"
    "} catch (e) {"
    "  if (e.codeName == 'AlreadyExists') "
    "    db.getSiblingDB('admin').changeUserPassword('root', process.env.MONGO_ROOT_PASSWORD);"
    "  else throw e"
    "}";

  return Crails::run_command("mongosh --quiet " + connection_string()
                             + " --eval '" + script + "'");
}

bool Wizard::enable_authorization()
{
  const std::string conf_path = get_mongod_conf_path();
  std::string current_conf;
  bool authorization_found = false;

  if (Crails::read_file(conf_path, current_conf))
  {
    if (current_conf.find("authorization:") != std::string::npos)
    {
      std::cerr << "authorization already enabled in " << conf_path << std::endl;
      authorization_found = true;
    }
  }
  if (!authorization_found)
  {
    std::ofstream stream(conf_path, std::ios::app);

    if (!stream.is_open())
    {
      std::cerr << "could not open " << conf_path << std::endl;
      return false;
    }
    stream << "\nsecurity:\n  authorization: enabled\n";
    stream.close();
  }
  return authorization_found || restart_service();
}

bool Wizard::authenticate_root() const
{
  std::string script =
    "if (!db.getSiblingDB('admin').auth('root', process.env.MONGO_ROOT_PASSWORD)) quit(1)";

  return Crails::run_command("mongosh --quiet " + connection_string()
                             + " --eval '" + script + "'");
}
