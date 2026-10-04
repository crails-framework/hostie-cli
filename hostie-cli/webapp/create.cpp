#include <crails/utils/random_string.hpp>
#include <filesystem>
#include <iostream>
#include "create.hpp"
#include "deployment.hpp"
#include "../service.hpp"
#include "../user.hpp"
#include "../databases/database.hpp"
#include "../hostie_variables.hpp"

using namespace std;
using namespace WebApp;

void CreateCommand::options_description(boost::program_options::options_description& options) const
{
  StandardCreator::options_description(options);
  options.add_options()
    ("database,D", boost::program_options::value<string>(), "database engine: postgres or mysql");
}

bool CreateCommand::initialize(int argc, const char** argv)
{
  if (!StandardCreator::initialize(argc, argv))
    return false;
  if (!options.count("port"))
  {
    cerr << "missing required option --port" << endl;
    return false;
  }
  if (!options.count("database"))
  {
    cerr << "missing required option --database (postgres or mysql)" << endl;
    return false;
  }

  const string engine = options["database"].as<string>();
  unique_ptr<Database> database = make_database(engine);

  if (!database)
  {
    cerr << "unsupported database engine `" << engine << "` (expected postgres or mysql)" << endl;
    return false;
  }

  const bool postgres = database->type() == "postgres";
  const string root_variable = postgres ? "postgres_root" : "mysql_root";

  if (!HostieVariables::global->has_variable(root_variable))
  {
    cerr << "no " << database->type() << " server set up: run `hostie-cli wizard "
         << (postgres ? "postgresql" : "mysql") << "` first" << endl;
    return false;
  }
  return true;
}

int CreateCommand::run()
{
  const string name = options["name"].as<string>();
  unique_ptr<Database> database = make_database(options["database"].as<string>());
  InstanceUser user;
  SystemService service;

  if (!load_user(user, options))
    return -1;
  if (!create_user(user) || !prepare_runtime_directory(user))
    return cancel(user, *database);

  service.app_name = name;
  service.app_user = user.name;
  service.app_group = user.group;
  database->configure(
    user.name, name,
    Crails::generate_random_string(database->password_charset(), 32)
  );

  // No service file here: crails-deploy generates it at deployment time.
  environment.set_variables({
    {"APPLICATION_NAME",  name},
    {"APPLICATION_USER",  user.name},
    {"APPLICATION_GROUP", user.group},
    {"APPLICATION_TYPE",  "WebApp"},
    {"APPLICATION_HOST",  "127.0.0.1"}, // only reachable to the local nginx instance
    {"APPLICATION_PORT",  to_string(options["port"].as<unsigned short>())},
    {"VAR_DIRECTORY",     var_directory.string()},
    {"DATABASE_URL",      database->get_url().to_string()}
  });

  if (prepare_environment_file() &&
      prepare_log_directory(service) &&
      database->prepare_user() &&
      database->prepare_database())
  {
    state += DatabaseCreated;
    if (install_dropin(Deployment(name)) && post_install_actions(*database))
    {
      print_next_steps();
      return 0;
    }
  }
  return cancel(user, *database);
}

bool CreateCommand::install_dropin(const Deployment& deployment)
{
  if (!Deployment::dropin_supported())
  {
    cerr << "warning: environment wiring is not supported on this platform yet:"
         << " the service deployed by crails-deploy won't read " << environment.get_path()
         << endl;
    return true;
  }
  if (!deployment.install_dropin(filesystem::absolute(environment.get_path())))
    return false;
  state += DropInCreated;
  return true;
}

void CreateCommand::print_next_steps() const
{
  const string name = options["name"].as<string>();

  cout
    << endl
    << "Instance `" << name << "` is ready for crails-deploy." << endl
    << "hostie-cli manages its user, runtime directory, database and environment;" << endl
    << "crails-deploy provides the service file. Variables set by crails-deploy" << endl
    << "take precedence over the ones set by hostie-cli." << endl
    << endl
    << "Example deployment (--start is specific to your application;" << endl
    << "add --env KEY=value for release-specific variables, --sudo if needed):" << endl
    << endl
    << "  " << deploy_example(environment) << endl
    << endl
    << "To print the matching options again:  hostie-cli webapp deploy-args -n " << name << endl
    << "To expose it through nginx:           hostie-cli nginx configure -n " << name << endl;
}

int CreateCommand::cancel(InstanceUser& user, Database& database)
{
  if ((state & DatabaseCreated) > 0)
    database.drop_database();
  if ((state & DropInCreated) > 0)
    Deployment(environment.get_project_name()).remove_dropin();
  return StandardCreator::cancel(user);
}
