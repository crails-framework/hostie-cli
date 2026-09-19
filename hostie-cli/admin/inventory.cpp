#include "inventory.hpp"
#include "../environment.hpp"
#include <algorithm>
#include <charconv>
#include <iostream>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct InstanceSpecs
{
  string type, name, var_directory;
  vector<string> domains;
  vector<pair<string, unsigned short>> ports;
};

struct PortVariable
{
  string role, variable;
};

static const map<string, vector<PortVariable>> extra_port_variables{
  {"Odoo", {{"gevent", "GEVENT_PORT"}}}
};

static optional<unsigned short> parse_port(const string& value)
{
  unsigned int port = 0;
  const char* end = value.data() + value.size();
  auto [ptr, error] = from_chars(value.data(), end, port);

  if (error != errc() || ptr != end || port == 0 || port > 65535)
    return nullopt;
  return static_cast<unsigned short>(port);
}

static void collect_port(InstanceSpecs& instance, const InstanceEnvironment& environment, const PortVariable& definition)
{
  const string value = environment.get_variable(definition.variable);

  if (value.empty())
    return ;
  if (auto port = parse_port(value))
    instance.ports.emplace_back(definition.role, *port);
  else
    cerr << instance.name << ": ignoring invalid " << definition.variable << " (" << value << ')' << endl;
}

static void collect_ports(InstanceSpecs& instance, const InstanceEnvironment& environment)
{
  auto extras = extra_port_variables.find(instance.type);

  collect_port(instance, environment, {"http", "APPLICATION_PORT"});
  if (extras != extra_port_variables.end())
  {
    for (const PortVariable& definition : extras->second)
      collect_port(instance, environment, definition);
  }
}

static vector<string> collect_domains(const InstanceEnvironment& environment)
{
  istringstream stream(environment.get_variable("HOSTIE_DOMAINS"));
  vector<string> domains;
  string domain;

  while (getline(stream, domain, ';'))
  {
    if (!domain.empty())
      domains.push_back(domain);
  }
  return domains;
}

static optional<InstanceSpecs> collect_instance(const filesystem::path& filepath)
{
  InstanceSpecs instance;
  InstanceEnvironment environment;

  if (filepath.extension() == ".env")
  {
    environment.set_project_name(instance.name);
    environment.load();
    instance.type = environment.get_variable("APPLICATION_TYPE");
    if (instance.type.size() > 0)
    {
      instance.name = filepath.stem().string();
      instance.var_directory = environment.get_variable("VAR_DIRECTORY");
      instance.domains = collect_domains(environment);
      collect_ports(instance, environment);
    }
    else
    {
      cerr << filepath << ": skipped (unreadable, or no APPLICATION_TYPE)" << endl;
      return {};
    }
    return instance;
  }
  return {};
}

static void output_instance(ostream& out, const InstanceSpecs& instance)
{
  out << "{\n    \"type\": " << std::quoted(instance.type)
      << ",\n    \"name\": " << std::quoted(instance.name)
      << ",\n    \"domains\": [";
  for (size_t i = 0 ; i < instance.domains.size() ; ++i)
  {
    if (i > 0) out << ", ";
    out << std::quoted(instance.domains[i]);
  }
  out << "],\n    \"var_directory\": " << std::quoted(instance.var_directory)
      << ",\n    \"ports\": {";
  for (size_t i = 0 ; i < instance.ports.size() ; ++i)
  {
    if (i > 0) out << ", ";
    out << std::quoted(instance.ports[i].first)
        << ": " << instance.ports[i].second;
  }
  out << "}\n  }";
}

int InventoryCommand::run()
{
  vector<filesystem::path> environments;
  vector<InstanceSpecs> instances;

  try
  {
    environments = instance_environments();
  }
  catch (const filesystem::filesystem_error& error)
  {
    cerr << "failed to list instances: " << error.what() << endl;
    return 1;
  }
  sort(environments.begin(), environments.end());
  for (const filesystem::path& filepath : environments)
  {
    if (auto instance = collect_instance(filepath))
      instances.push_back(std::move(*instance));
  }
  cout << "[\n";
  for (size_t i = 0 ; i < instances.size() ; ++i)
  {
    output_instance(cout, instances[i]);
    cout << (i + 1 < instances.size() ? ",\n" : "\n");
  }
  cout << "]\n";
  return 0;
}
