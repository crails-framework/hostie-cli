#include "fedora.hpp"
#include "../postgres/fedora.hpp"
#include <crails/cli/process.hpp>
#include <iostream>

using namespace std;
using namespace Odoo::Fedora;

int Wizard::run()
{
  requirements.push_back("odoo");
  if (require_wizard<FedoraPostgresWizard>())
  {
    string add_repo_command = "dnf config-manager --add-repo=https://nightly.odoo.com/17.0/nightly/rpm/odoo.repo";

    if (system(add_repo_command.c_str()) == 0)
    {
      if (install_requirements())
      {
        return Odoo::Wizard::run();
      }
    }
    else
      cerr << "failed to add odoo community repository" << endl;
  }
  return -1;
}
