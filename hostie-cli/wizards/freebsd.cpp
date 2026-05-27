#include "freebsd.hpp"
#include <sstream>

using namespace std;

bool FreeBSDWizard::install_requirements()
{
  stringstream command;

  command << "pkg install -y";
  for (const string_view package : requirements)
    command << ' ' << quoted(package);
  return system(command.str().c_str()) == 0 && install_package_from_urls();
}
