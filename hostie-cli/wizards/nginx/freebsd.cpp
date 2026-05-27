#include "freebsd.hpp"
#include <crails/cli/process.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>

using namespace std;
using namespace Nginx::FreeBSD;

int Wizard::run()
{
  store.variable("web-user", "www");
  store.variable("web-group", "www");
  store.variable("nginx-root", "/usr/local/etc/nginx");
  requirements.push_back("nginx");
  requirements.push_back("py311-certbot");
  requirements.push_back("py311-certbot-nginx");
  if (install_requirements() && prepare_conf() && start_service())
    return Nginx::Wizard::run();
  return -1;
}

bool Wizard::start_service()
{
  return Crails::run_command("service nginx onestart")
      && Crails::run_command("sysrc nginx_enabled=\"YES\"");
}

bool Wizard::prepare_conf()
{
  const string_view default_conf(
    "user www;\n"
    "worker_processes auto;\n"
    "include /usr/local/etc/nginx/modules-enabled/*.conf;\n"
    "\n"
    "events {\n"
    "  worker_connections 1024;\n"
    "}\n\n"
    "http {\n"
    "  include mime.types;\n"
    "  default_type application/octet-stream;\n"
    "  sendfile on;\n"
    "  keepalive_timeout 65;\n"
    "  gzip on;\n"
    "\n"
    "  include /usr/local/etc/nginx/sites-enabled/*;\n"
    "}\n\n"
  );
  ofstream stream("/usr/local/etc/nginx/nginx.conf", ios::trunc);

  if (stream.is_open())
  {
    stream << default_conf;
    stream.close();
    if (filesystem::create_directories("/usr/local/etc/nginx/sites-enabled"))
      return true;
    else
      cerr << "Failed to create sites-enabled directory" << endl;
  }
  else
    cerr << "Could not open /usr/local/etc/nginx/nginx.conf" << endl;
  return false;
}
