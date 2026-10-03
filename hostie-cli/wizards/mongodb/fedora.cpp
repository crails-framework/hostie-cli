#include "fedora.hpp"
#include <crails/cli/process.hpp>
#include <crails/cli/filesystem.hpp>
#include <iostream>

using namespace std;
using namespace MongoDB::Fedora;

static const char* get_redhat_version()
{
  const char* value = std::getenv("REDHAT_VERSION");
  return value ? value : "10";
}

int Wizard::run()
{
  string repo_path = "/etc/yum.repos.d/mongodb-org-" + get_mongodb_version() + ".repo";
  string repo_source =
    "[mongodb-org-" + get_mongodb_version() + "]\n"
    "name=MongoDB Repository\n"
    "baseurl=https://repo.mongodb.org/yum/redhat/" + get_redhat_version() + "/mongodb-org/" + get_mongodb_version() + "/x86_64/\n"
    "gpgcheck=1\n"
    "enabled=1\n"
    "gpgkey=https://www.mongodb.org/static/pgp/server-" + get_mongodb_version() + ".asc\n";

  requirements.push_back("mongodb-org");
  if (Crails::write_file("MongoDB wizard", repo_path, repo_source))
  {
    if (install_requirements() && start_service())
      return MongoDB::Wizard::run();
  }
  return -1;
}

bool Wizard::start_service()
{
  return Crails::run_command("systemctl enable mongod")
      && Crails::run_command("systemctl start mongod");
}
