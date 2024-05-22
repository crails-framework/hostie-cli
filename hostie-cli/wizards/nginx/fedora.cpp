#include "fedora.hpp"
#include <crails/cli/process.hpp>

using namespace std;
using namespace Nginx::Fedora;

int Wizard::run()
{
  store.variable("web-user", "nginx");
  store.variable("web-group", "www-data");
  requirements.push_back("nginx");
  requirements.push_back("certbot");
  if (install_requirements() && start_service())
    return Nginx::Wizard::run();
  return -1;
}

bool Wizard::start_service()
{
  return Crails::run_command("systemctl enable nginx")
      && Crails::run_command("systemctl start nginx");
}
