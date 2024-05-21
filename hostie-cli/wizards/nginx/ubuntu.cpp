#include "ubuntu.hpp"
#include <crails/cli/process.hpp>

using namespace std;
using namespace Nginx::Ubuntu;

int Wizard::run()
{
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
