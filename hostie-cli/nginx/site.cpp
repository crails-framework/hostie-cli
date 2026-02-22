#include "site.hpp"
#include <crails/cli/process.hpp>
#include <sstream>
#include <iostream>
#include <fstream>
#define ind(N) string((N) * 2, ' ') <<

using namespace std;
using namespace Nginx;
using namespace HttpServer;

void ConfigureSite::options_description(boost::program_options::options_description& options) const
{
  InstanceCommand::options_description(options);
  options.add_options()
    ("certify", "create or update SSL certificates using letsencrypt");
}

static string php_version()
{
  string php_version;
  string getter = "php --version | grep -e '^PHP' | cut -d' ' -f2 | cut -d'-' -f1 | cut -d'.' -f1,2";
  ostringstream command;

  command << "sh -c " << quoted(getter);
  Crails::run_command(command.str(), php_version);
  return php_version.substr(0, php_version.length() - 1);
}

static string location_custom_settings(const Location& location)
{
  ostringstream stream;

  for (const string& line : location.custom_settings)
    stream << ind(2) line << endl;
  return stream.str();
}

static string server_custom_settings(const Site& site)
{
  ostringstream stream;

  for (const string& line : site.custom_settings)
    stream << ind(1) line << endl;
  return stream.str();
}

string ConfigureSite::location_custom_pass(const Location& location, bool ssl)
{
  ostringstream stream;
  stream
  << ind(1) "location " << location.path << " {" << endl
  << location_custom_settings(location)
  << ind(1) "}" << endl;
  return stream.str();
}

string ConfigureSite::location_directory_pass(const Location& location, bool ssl)
{
  ostringstream stream;
  stream
  << ind(1) "location " << location.path << " {" << endl
  << ind(2) "root " << location.target << ';' << endl
  << location_custom_settings(location)
  << ind(1) "}" << endl;
  return stream.str();
}

string ConfigureSite::location_phpfpm_pass(const Location& location, bool ssl)
{
  ostringstream stream;
  stream
  << ind(1) "location " << location.path << " {" << endl
  << ind(2) "index index.html index.htm index.php;" << endl
  << ind(2) "try_files $uri $uri/ /index.php$is_args$args;" << endl
  << ind(2) "location ~ \\.php$ {" << endl
  << ind(3) "fastcgi_pass unix:" << environment.get_variable("PHP_FPM_SOCKET") <<';' << endl
  << ind(3) "fastcgi_index index.php;" << endl
  << ind(3) "fastcgi_param App-Root " << location.target << ';' << endl
  << ind(3) "fastcgi_param ABSPATH " << location.target << ';' << endl
  << ind(3) "include fastcgi.conf;" << endl
  << location_custom_settings(location)
  << ind(2) '}' << endl
  << ind(1) '}' << endl;
  return stream.str();
}

string ConfigureSite::location_proxy_pass(const Location& location, bool ssl)
{
  ostringstream stream;
  stream
  << ind(1) "location " << location.path << " {" << endl
  << ind(2) "proxy_pass http://" << location.target << ';' << endl
  << ind(2) "proxy_set_header Host              $host;" << endl
  << ind(2) "proxy_set_header X-Real-IP         $remote_addr;" << endl
  << ind(2) "proxy_set_header X-Forwarded-For   $proxy_add_x_forwarded_for;" << endl
  << ind(2) "proxy_set_header X-Forwarded-Proto $scheme;" << endl
  << ind(2) "proxy_set_header X-Forwarded-Ssl   " << (ssl ? "on" : "off") << ";" << endl
  << ind(2) "proxy_set_header X-Forwarded-Port  $server_port;" << endl
  << ind(2) "proxy_set_header X-Forwarded-Host  $host;" << endl
  << location_custom_settings(location)
  << ind(1) '}' << endl;
  return stream.str();
}

string ConfigureSite::location_forbidden(const Location& location)
{
  ostringstream stream;

  stream
  << ind(1) "location" << location.path << '{' << endl
  << ind(2) "return 403;" << endl
  << ind(1) '}' << endl;
  return stream.str();
}

string ConfigureSite::location_redirect(const Location& location)
{
  ostringstream stream;

  stream
  << ind(1) "location " << location.path << '{' << endl
  << ind(2) "return 301 " << location.target << ';' << endl
  << ind(1) '}' << endl;
  return stream.str();
}

string ConfigureSite::location_https_redirect(const Location& location)
{
  return location_redirect(Location{
    location.path, "https://$server_name$request_uri"
  });
}

string ConfigureSite::server_locations(bool ssl, bool certified)
{
  ostringstream stream;

  for (const Location& location : site.locations)
  {
    if ((ssl && (location.ssl & SslOmit) > 0) ||
        (!ssl && (location.ssl & SslOnly) > 0 && certified))
    {
      continue;
    }
    else if ((location.ssl & SslRequired) && !ssl && certified)
    {
      stream << location_https_redirect(location);
    }
    else
    {
      switch (location.type)
      {
      case DirectoryLocation:
        stream << location_directory_pass(location, ssl);
        break ;
      case AppProxyLocation:
        stream << location_proxy_pass(location, ssl);
        break ;
      case PhpFpmLocation:
        stream << location_phpfpm_pass(location, ssl);
        break ;
      case RedirectLocation:
        stream << location_redirect(location);
        break ;
      case ForbiddenLocation:
        stream << forbidden_location(location, ssl);
        break ;
      case CustomLocation:
        stream << location_custom_pass(location, ssl);
        break ;
      }
    }
  }
  return stream.str();
}

string ConfigureSite::server_common_conf(const string_view domain_name)
{
  ostringstream stream;

  stream
    << ind(1) "server_name " << domain_name << ';' << endl
    << ind(1) "root " << site.var_directory.string() << ';' << endl
    << ind(1) "include /etc/nginx/standard-error-pages.conf*;" << endl
    << ind(1) "include /etc/nginx/letsencrypt.conf*;" << endl
    << server_custom_settings(site);
  return stream.str();
}

string ConfigureSite::server_https_conf(const string_view domain_name)
{
  if (is_domain_certified(domain_name))
  {
    ostringstream stream;
    filesystem::path fullchain_path = certificate_directory(domain_name) / "fullchain.pem";
    filesystem::path privkey_path = certificate_directory(domain_name) / "privkey.pem";
    std::string listen_options = "ssl";

    if (site.protocol.length())
      listen_options += ' ' + site.protocol;

    stream
      << "server {" << endl
      << ind(1) "listen 443 " << listen_options << ';' << endl
      << ind(1) "listen [::]:443 " << listen_options << ';' << endl
      << ind(1) "ssl_certificate " << fullchain_path.string() << ';' << endl
      << ind(1) "ssl_certificate_key " << privkey_path.string() << ';' << endl
      << server_common_conf(domain_name) << endl
      << server_locations(true, true)
      << '}' << endl
      << endl;
    return stream.str();
  }
  return string();
}

string ConfigureSite::server_http_conf(const string_view domain_name)
{
  ostringstream stream;
  bool ssl_enabled = is_domain_certified(domain_name);
  std::string listen_options;

  if (site.protocol.length())
    listen_options += ' ' + site.protocol;

  stream
    << "server {" << endl
    << ind(1) "listen 80" << listen_options << ';' << endl
    << ind(1) "listen [::]:80" << listen_options << ';' << endl
    << server_common_conf(domain_name) << endl
    << server_locations(false, ssl_enabled)
    << '}' << endl
    << endl;
  return stream.str();
}

string ConfigureSite::server_conf(const string_view domain_name)
{
  ostringstream stream;

  stream
    << server_http_conf(domain_name) << endl
    << server_https_conf(domain_name);
  return stream.str();
}

string ConfigureSite::site_conf()
{
  ostringstream stream;

  for (const Upstream& upstream : site.upstreams)
  {
    stream
    << "upstream " << upstream.name << " {" << endl
    << "  server " << upstream.ip << ':' << upstream.port << ';' << endl
    << '}' << endl << endl;
  }
  stream
    << "map $http_upgrade $connection_upgrade {" << endl
    << "  default upgrade;" << endl
    << "  '' close;" << endl
    << '}' << endl;
  for (const string& domain_name : domain_names())
  {
    stream
      << endl
      << server_conf(domain_name);
  }
  return stream.str();
}

int ConfigureSite::make_site()
{
  ofstream stream(site_conf_path());

  if (stream.is_open())
  {
    stream << site_conf();
    stream.close();
    if (system("systemctl reload nginx") == 0)
      return 0;
    cerr << "failed to reload nginx" << endl;
    return 11;
  }
  else
    cerr << "could not open " << site_conf_path() << endl;
  return 10;
}

int ConfigureSite::run()
{
  int status = make_site();

  if (status == 0 && options.count("certify"))
  {
    if (!renew_certificates())
    {
      cerr << "failed to renew certificates" << endl;
      return 20;
    }
  }
  return status;
}

