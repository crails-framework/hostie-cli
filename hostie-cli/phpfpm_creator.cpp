#include "phpfpm_creator.hpp"
#include "user.hpp"
#include "hostie_variables.hpp"
#include "php.hpp"
#include <crails/read_file.hpp>
#include <crails/utils/split.hpp>
#include <fstream>

using namespace std;

filesystem::path PhpFpmCreator::find_php_source(const std::string& name, const char* env_key) const
{
  const std::string varname = name + "-source";

  if (options.count(varname))
    return filesystem::weakly_canonical(options[varname].as<string>());
  else if (std::getenv(env_key) != 0)
    return filesystem::weakly_canonical(getenv(env_key));
  else if (HostieVariables::global->has_variable(varname))
    return filesystem::weakly_canonical(HostieVariables::global->variable(varname));
  cerr << "could not deduce " << name << " directory" << endl;
  return filesystem::path();
}

filesystem::path PhpFpmCreator::find_php_fpm_socket_path(const filesystem::path& fpm_conf_path)
{
  filesystem::path conf = fpm_conf_path.parent_path() / "www.conf";
  filesystem::path result;
  string source;
  
  // Scanning www.conf to use a modified version of the default socket path
  if (Crails::read_file(conf, source))
  {
    auto parts = Crails::split<string_view, vector<string_view>>(string_view(source), '\n');
    auto it = find_if(parts.begin(), parts.end(), [](const string_view line) -> bool
    {
      return line.find("listen = ") == 0;
    });

    if (it != parts.end())
    {
      result = it->substr(9);
      result = result.replace_filename(
        result.stem().string() + '-' + environment.get_project_name() + result.extension().string()
      );
    }
  }
  // Falling back to a debian-compatible approach
  if (result.empty())
    result = "/run/php/php" + php_version() + "-fpm-" + environment.get_project_name() + ".sock";
  environment.set_variable("PHP_FPM_SOCKET", result.string());
  return result;
}

bool PhpFpmCreator::generate_fpm_pool(const InstanceUser& user)
{
  filesystem::path fpm_conf_path = fpm_pool_path(environment);
  ofstream stream(fpm_conf_path);

  if (stream.is_open())
  {
    stream
      << '[' << environment.get_project_name() << ']' << '\n'
      << "user = " << user.name << '\n'
      << "group = " << user.group << '\n'
      << "listen = " << find_php_fpm_socket_path(fpm_conf_path).string() << '\n'
      << "listen.owner = " << HostieVariables::global->variable("web-user")<< '\n'
      << "listen.group = " << HostieVariables::global->variable("web-group") << '\n'
      << "chdir = " << var_directory.string() << '\n'
      << "pm = ondemand\n"
      << "pm.max_children = 4\n";
    stream.close();
    state += FpmPoolCreated;
    return true;
  }
  else if (fpm_conf_path.empty())
    cerr << "failed to deduce php-fpm conf directory" << endl;
  else
    cerr << "failed to create php fpm pool at " << fpm_conf_path << endl;
  return false;
}

int PhpFpmCreator::cancel(InstanceUser& user)
{
  if ((state & FpmPoolCreated) > 0)
    filesystem::remove(fpm_pool_path(environment));
  return StandardCreator::cancel(user);
}
