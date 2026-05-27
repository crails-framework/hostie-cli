#include "list_command.hpp"
#include "root.hpp"
#include <filesystem>
#include <iostream>

using namespace std;
using namespace Nginx;

int ListCommand::run()
{
  filesystem::path path = nginx_root_path() / "sites-enabled";

  if (filesystem::is_directory(path))
  {
    for (const auto& entry : filesystem::directory_iterator(path))
    {
      filesystem::path site_path = entry.path();

      if (site_path.extension().string() == ".hostie")
        cout << site_path.stem().string() << endl;
    }
  }
  return 0;
}
