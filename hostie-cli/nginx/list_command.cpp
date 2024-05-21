#include "list_command.hpp"
#include <filesystem>
#include <iostream>

using namespace std;
using namespace Nginx;

int ListCommand::run()
{
  filesystem::path path("/etc/nginx/sites-enabled");

  for (const auto& entry : filesystem::directory_iterator(path))
  {
    filesystem::path site_path = entry.path();

    if (site_path.extension().string() == ".hostie")
      cout << site_path.stem().string() << endl;
  }
  return 0;
}
