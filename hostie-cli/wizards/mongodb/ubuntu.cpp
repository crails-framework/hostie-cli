#include "ubuntu.hpp"
#include <crails/cli/process.hpp>
#include <iostream>

using namespace std;
using namespace MongoDB::Ubuntu;

int Wizard::run()
{
  requirements.push_back("mongodb-org");
  if (Crails::require_command("curl"))
  {
    filesystem::path keyring_path("/usr/share/keyrings/mongodb-archive-keyring.gpg");
    string fetch_key_command = "curl -fsSL https://www.mongodb.org/static/pgp/server-" + get_mongodb_version() + ".asc | "
                               "gpg --dearmor -o " + keyring_path.string();

    if (system(fetch_key_command.c_str()) == 0)
    {
      string add_source_command =
        "echo 'deb [arch=amd64,arm64 signed-by=" + keyring_path.string() + "] "
        "https://repo.mongodb.org/apt/ubuntu " + release_codename() + "/mongodb-org/" + get_mongodb_version() + " multiverse' |"
        "sudo tee /etc/apt/sources.list.d/mongodb-org-" + get_mongodb_version() + ".list";

      if (system(add_source_command.c_str()) == 0)
      {
        if (system("apt-get update -y") == 0)
        {
          if (install_requirements() && start_service())
          {
            return MongoDB::Wizard::run();
          }
        }
      }
    }
  }
  return -1;
}

bool Wizard::start_service()
{
  return Crails::run_command("systemctl enable mongod")
      && Crails::run_command("systemctl start mongod");
}
