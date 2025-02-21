#include "domain_name.hpp"
#include <crails/utils/semantics.hpp>
#include <crails/utils/split.hpp>
#include <cstdlib>

using namespace std;
using namespace Nginx;
using namespace HttpServer;

bool DomainNameCommand::initialize(int argc, const char** argv)
{
  if (InstanceCommand::initialize(argc, argv))
  {
    environment.load();
    string application_type = environment.get_variable("APPLICATION_TYPE");
    auto initializer = Site::initializers.find(application_type);

    if (initializer != Site::initializers.end())
    {
      site.name = Crails::underscore(environment.get_project_name());
      site.var_directory = environment.get_variable("VAR_DIRECTORY");
      initializer->second(environment, site);
      return true;
    }
    else
      cerr << "HttpServer: application type " << application_type << " not supported." << endl;
  }
  return false;
}

string DomainNameCommand::webmaster_email() const
{
  string value = environment.get_variable("WEBMASTER_EMAIL");

  if (value.length() == 0)
  {
    const char* env_variable = std::getenv("WEBMASTER_EMAIL");
    if (env_variable)
      value = string(env_variable);
  }
  return value;
}

filesystem::path DomainNameCommand::site_conf_path() const
{
  return filesystem::path("/etc/nginx/sites-enabled") / (site.name + ".hostie");
}
