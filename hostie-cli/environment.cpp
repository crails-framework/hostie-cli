#include <crails/read_file.hpp>
#include <crails/utils/split.hpp>
#include <crails/utils/join.hpp>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include "environment.hpp"

using namespace std;

filesystem::path InstanceEnvironment::get_root_path()
{
  const char* farm_path = std::getenv("CRAILSCMS_FARM_PATH");
  
  return filesystem::canonical(
    farm_path ? filesystem::path(farm_path) : filesystem::current_path()
  );
}

void InstanceEnvironment::set_project_name(const string& value)
{
  const string filename = value + ".env";

  project_name = value;
  environment_path = get_root_path() / filename;
}
  
std::string InstanceEnvironment::get_variable(const std::string& name) const
{
  auto it = variables.find(name);
  return it != variables.end() ? it->second : string();
}

void InstanceEnvironment::set_variables(const std::map<std::string,std::string>& values)
{
  for (auto it = values.begin() ; it != values.end() ; ++it)
    set_variable(it->first, it->second);
}

void InstanceEnvironment::set_variable(const std::string& name, const std::string& value)
{
  variables[name] = value;
  setenv(name.c_str(), value.c_str(), 1);
}

void InstanceEnvironment::append(const string_view contents)
{
  auto lines = Crails::split(contents, '\n');

  for (const string_view line : lines)
  {
    auto parts = Crails::split(line, '=');
    if (parts.size() >= 2)
    {
      string name = string(*parts.begin());
      string part2 = Crails::join(++parts.begin(), parts.end(), '=');
      string value;

      if (part2[0] == '"' || part2[0] == '\'')
      {
        stringstream value_stream;
        value_stream << part2;
        value_stream >> quoted(value, part2[0]);
      }
      else
        value = part2;
      variables.insert_or_assign(name, value);
    }
  }
}

void InstanceEnvironment::load()
{
  string contents;

  if (Crails::read_file(environment_path.string(), contents))
    append(string_view(contents.c_str(), contents.length()));
}

bool InstanceEnvironment::save()
{
  ofstream stream(environment_path.string());

  if (stream.is_open())
  {
    stream << "#/bin/sh\n";
    for (auto it = variables.begin() ; it != variables.end() ; ++it)
      stream << it->first << '=' << quoted(it->second) << '\n';
    stream.close();
    return true;
  }
  return false;
}
