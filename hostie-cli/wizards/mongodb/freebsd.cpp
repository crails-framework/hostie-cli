#include "freebsd.hpp"
#include <crails/cli/process.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace MongoDB::FreeBSD;

int Wizard::run()
{
  string package_name = "mongodb";
  string mongodb_version = get_mongodb_version();

  if (mongodb_version.length() == 3)
  {
    package_name += mongodb_version[0];
    package_name += mongodb_version[2];
    requirements.push_back(package_name);
    if (install_requirements() && prepare_conf() && start_service())
      return MongoDB::Wizard::run();
  }
  else
    cerr << "expected mongodb version to be 3 characters (like `9.0`)" << endl;
  return -1;
}

bool Wizard::start_service()
{
  return Crails::run_command("service mongod onestart")
      && Crails::run_command("sysrc mongod_enable=\"YES\"");
}

bool Wizard::prepare_conf()
{
  const string_view default_conf(
    "systemLog:\n"
    "  destination: file\n"
    "  logAppend: true\n"
    "  path: /var/log/mongodb/mongod.log\n"
    "\n"
    "storage:\n"
    "  dbPath: /var/db/mongodb\n"
    "\n"
    "processManagement:\n"
    "  timeZoneInfo: /usr/share/zoneinfo\n"
    "\n"
    "net:\n"
    "  port: 27017\n"
    "  bindIp: 127.0.0.1\n"
  );
  ofstream stream("/usr/local/etc/mongod.conf", ios::trunc);

  if (stream.is_open())
  {
    stream << default_conf;
    stream.close();
    Crails::run_command("mkdir -p /var/db/mongodb /var/log/mongodb");
    Crails::run_command("chown mongodb:mongodb /var/db/mongodb /var/log/mongodb");
    return true;
  }
  else
    cerr << "Could not open /usr/local/etc/mongod.conf" << endl;
  return true;
}
