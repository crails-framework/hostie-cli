#include "hostie_variables.hpp"
#include <filesystem>
#include <crails/read_file.hpp>
#include <cstdlib>

using namespace std;

static string store_path()
{
  const char* from_env = getenv("HOSTIE_RC");

  return from_env ? from_env : "/etc/hostie.rc";
}

HostieVariables::HostieVariables() : Crails::ProjectVariables(store_path())
{
  initialize();
}
