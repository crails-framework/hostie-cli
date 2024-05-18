#include <crails/read_file.hpp>
#include <iostream>
#include "list_command.hpp"
#include "environment.hpp"

using namespace std;

int ListCommand::run()
{
  for (const auto& filepath : instance_environments())
  {
    if (should_display_entry(filepath))
    {
      cout
        << filepath.filename().replace_extension().string()
        << endl;
    }
  }
  return 0;
}

vector<filesystem::path> ListCommand::instance_environments()
{
  auto it = filesystem::directory_iterator{
    InstanceEnvironment::get_root_path()
  };
  vector<filesystem::path> list;

  for (const auto& entry : it)
  {
    filesystem::path filepath = entry.path();

    if (!filesystem::is_directory(filepath))
      list.push_back(filepath);
  }
  return list;
}

bool ListCommand::has_environment_type(const filesystem::path& filepath, const string_view type)
{
  string environment;

  if (Crails::read_file(filepath.string(), environment))
  {
    stringstream stream;

    stream << "APPLICATION_TYPE=\"" << type << '"';
    return environment.find(stream.str()) != string::npos;
  }
  return false;
}
