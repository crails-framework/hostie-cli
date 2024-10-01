#include "list_command.hpp"
#include "../environment.hpp"
#include <iostream>

using namespace std;
using namespace Wordpress;

void WpListCommand::options_description(boost::program_options::options_description& options) const
{
  options.add_options()
    ("versions,v", "show wordpress versions in use");
}

void WpListCommand::display_entry(const filesystem::path& filepath) const
{
  if (options.count("versions"))
    display_with_version(filepath);
  else
    ListCommand::display_entry(filepath);
}

void WpListCommand::display_with_version(const filesystem::path& filepath) const
{
  const string name = filepath.filename().replace_extension().string();
  InstanceEnvironment environment;

  environment.set_project_name(name);
  environment.load();
  cout
    << environment.get_variable("WORDPRESS_VERSION") << '\t' << name
    << endl;
}
