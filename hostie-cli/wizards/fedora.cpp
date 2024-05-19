#include "fedora.hpp"
#include <sstream>

using namespace std;

bool FedoraWizard::install_requirements()
{
  stringstream command;

  command << "dnf install -y";
  for (const string_view package : requirements)
    command << ' ' << quoted(package);
  return system(command.str().c_str()) == 0;
}
