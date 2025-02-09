#include "ubuntu.hpp"
#include <crails/cli/filesystem.hpp>
#include <crails/cli/process.hpp>
#include <sstream>
#include <iostream>

using namespace std;

bool UbuntuWizard::install_requirements()
{
  stringstream command;

  command << "apt-get install -y";
  for (const string_view package : requirements)
    command << ' ' << quoted(package);
  return system(command.str().c_str()) == 0 && install_package_from_urls();
}

bool UbuntuWizard::add_repository(const string& name, const string_view list, const filesystem::path& key)
{
  static const filesystem::path list_root("/etc/apt/sources.list.d");
  static const filesystem::path gpg_root("/etc/apt/trusted.gpg.d");
  stringstream list_contents;
  filesystem::path target = list_root / (name + ".list");
  filesystem::path key_target =gpg_root / (name + ".gpg");

  if (filesystem::exists(key_target)) filesystem::remove(key_target);
  Crails::move_file(key, key_target);
  list_contents << "deb [signed-by=" << key_target.string() << "]"
                << ' ' << list;
  if (Crails::write_file("add_repository", target.string(), list_contents.str()))
    return Crails::run_command("apt-get update");
  else
    cerr << "cannot write into file " << target << endl;
  return false;
}
