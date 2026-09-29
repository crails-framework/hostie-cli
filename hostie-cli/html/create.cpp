#include "create.hpp"
#include "../user.hpp"

using namespace Html;
using namespace std;

int CreateCommand::run()
{
  InstanceUser user;

  if (!load_user(user, options))
    return -1;
  user.group = HostieVariables::global->variable("web-group");
  if (!create_user(user) ||
      !prepare_runtime_directory(user))
    return cancel(user);

  environment.set_variables({
    {"APPLICATION_NAME", options["name"].as<string>()},
    {"APPLICATION_USER", user.name},
    {"APPLICATION_TYPE", "HTML"},
    {"VAR_DIRECTORY",    var_directory.string()}
  });

  if (prepare_environment_file())
    return 0;
  return cancel(user);
}
