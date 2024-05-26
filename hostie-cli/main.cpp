#include <boost/program_options.hpp>
#include <crails/read_file.hpp>
#include <filesystem>
#include <iostream>
#include <crails/cli/command_index.hpp>
#include <crails/cli/process.hpp>
#include <crails/cli/with_path.hpp>
#include "crailscms/index.hpp"
#include "wordpress/index.hpp"
#include "odoo/index.hpp"
#include "nextcloud/index.hpp"
#include "wizards/index.hpp"
#include "nginx/index.hpp"
#include "hostie_variables.hpp"

using namespace std;

bool sudo = false;

int main(int argc, const char** argv)
{
  HostieVariables::global = make_unique<HostieVariables>();
  Crails::CommandIndex index;
  Crails::WithPath path_lock(HostieVariables::global->variable_or("hostie-root-path", "/var/lib/hosties"));

  index.add_command("crailscms", []() { return std::make_shared<CrailsCmsIndex>(); });
  index.add_command("wordpress", []() { return std::make_shared<WordpressIndex>(); });
  index.add_command("odoo",      []() { return std::make_shared<OdooIndex>(); });
  index.add_command("nextcloud", []() { return std::make_shared<NextCloudIndex>(); });
  index.add_command("nginx", []() { return std::make_shared<Nginx::IndexCommand>(); });
  index.add_command("wizard", []() { return std::make_shared<WizardsIndex>(); });
  if (index.initialize(argc, argv))
    return index.run();
  return -1;
}
