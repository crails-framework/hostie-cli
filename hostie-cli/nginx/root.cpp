#include "root.hpp"
#include "../hostie_variables.hpp"

using namespace std;

filesystem::path nginx_root_path()
{
  HostieVariables store;

  return store.variable_or("nginx-root", "/etc/nginx");
}
