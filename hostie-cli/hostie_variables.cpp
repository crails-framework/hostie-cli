#include "hostie_variables.hpp"
#include <filesystem>
#include <cstdlib>
#include <iomanip>
#include <sstream>

using namespace std;

unique_ptr<HostieVariables> HostieVariables::global;

static string store_path()
{
  const char* from_env = getenv("HOSTIE_RC");

  return from_env ? from_env : "/etc/hostie.rc";
}

static string touch_store_command()
{
  ostringstream stream;
  stream << "touch " << quoted(store_path());
  return stream.str();
}

HostieVariables::HostieVariables() : Crails::ProjectVariables(store_path())
{
  system(touch_store_command().c_str());
  initialize();
}

void HostieVariables::save()
{
  Crails::ProjectVariables::save();
  filesystem::permissions(store_path(), filesystem::perms::owner_read | filesystem::perms::owner_write);
}
