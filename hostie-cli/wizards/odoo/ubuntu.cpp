#include "ubuntu.hpp"
#include "../postgres/ubuntu.hpp"
#include <crails/cli/process.hpp>
#include <iostream>

using namespace std;
using namespace Odoo::Ubuntu;

int Wizard::run()
{
  requirements.push_back("odoo");
  if (Crails::require_command("curl") && require_wizard<UbuntuPostgresWizard>())
  {
    filesystem::path keyring_path("/usr/share/keyrings/odoo-archive-keyring.gpg");
    string fetch_key_command = "curl https://nightly.odoo.com/odoo.key | "
                               "gpg --dearmor -o " + keyring_path.string();

    if (system(fetch_key_command.c_str()) == 0)
    {
      string add_source_command =
        "echo 'deb [signed-by=" + keyring_path.string() + "] "
        "https://nightly.odoo.com/17.0/nightly/deb/ ./' | "
        "sudo tee /etc/apt/sources.list.d/odoo.list";

      if (system(add_source_command.c_str()) == 0)
      {
        if (system("apt-get update -y") == 0)
        {
          if (install_requirements())
          {
            return Odoo::Wizard::run();
          }
        }
      }
    }
  }
  return -1;
}

int Wizard_2404::run()
{
  requirements.push_back("python3-lxml-html-clean");
  return Odoo::Ubuntu::Wizard::run();
}
