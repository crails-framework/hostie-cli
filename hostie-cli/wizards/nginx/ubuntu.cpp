#include "ubuntu.hpp"
#include <crails/cli/process.hpp>

using namespace std;
using namespace Nginx::Ubuntu;

int Wizard::run()
{
  store.variable("web-user", "www-data");
  store.variable("web-group", "www-data");
  requirements.push_back("nginx");
  requirements.push_back("certbot");
  requirements.push_back("python3-certbot-nginx");
  if (install_requirements() && start_service())
    return Nginx::Wizard::run();
  return -1;
}

bool Wizard::start_service()
{
  return Crails::run_command("systemctl enable nginx")
      && Crails::run_command("systemctl start nginx");
}
