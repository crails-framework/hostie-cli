#include <boost/program_options.hpp>
#include <crails/read_file.hpp>
#include <filesystem>
#include <iostream>
#include <crails/cli/command_index.hpp>
#include <crails/cli/process.hpp>
#include "crailscms/index.hpp"
#include "wordpress/index.hpp"
#include "odoo/index.hpp"
#include "wizards/index.hpp"
#include "nginx/index.hpp"
#include "hostie_variables.hpp"

using namespace std;

filesystem::path crailscms_bin_dir;
bool sudo = false;

static filesystem::path make_crailscms_bin_dir(const char* argv0)
{
  error_code ec;
  filesystem::path current_bin_dir = filesystem::canonical(filesystem::path(argv0), ec);

  if (ec)
    return Crails::which(argv0);
  return current_bin_dir.parent_path().parent_path().parent_path();
}

int main(int argc, const char** argv)
{
  Crails::CommandIndex index;

  crailscms_bin_dir = make_crailscms_bin_dir(argv[0]);
  HostieVariables::global = make_unique<HostieVariables>();

  index.add_command("crailscms", []() { return std::make_shared<CrailsCmsIndex>(); });
  index.add_command("wordpress", []() { return std::make_shared<WordpressIndex>(); });
  index.add_command("odoo",      []() { return std::make_shared<OdooIndex>(); });
  //index.add_command("nextcloud", []() { return nullptr; });
  index.add_command("nginx", []() { return std::make_shared<Nginx::IndexCommand>(); });
  index.add_command("wizard", []() { return std::make_shared<WizardsIndex>(); });
  if (index.initialize(argc, argv))
    return index.run();
  return -1;
}
