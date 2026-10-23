#include "plugin_copy.hpp"
#include <crails/cli/process.hpp>
#include "temporary_directory.hpp"
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;

vector<string> Wordpress::copy_missing_plugins(const fs::path& from_plugins, const fs::path& to_plugins, vector<string>& errors)
{
  vector<string> copied, names;
  error_code error;

  if (!fs::is_directory(from_plugins, error) || !fs::is_directory(to_plugins, error))
    return copied;
  for (const auto& entry : fs::directory_iterator(from_plugins, error))
  {
    const string name = entry.path().filename().string();

    if (!name.empty() && name[0] != '.' && name != "index.php")
      names.push_back(name);
  }
  sort(names.begin(), names.end());
  for (const string& name : names)
  {
    TemporaryDirectory work(to_plugins);

    if (fs::symlink_status(to_plugins / name, error).type() != fs::file_type::not_found)
      continue ;
    if (!work || !Crails::run_command({"cp", {"-a", (from_plugins / name).string(), (work.path() / name).string()}}))
    {
      errors.push_back("could not copy " + name);
      continue ;
    }
    fs::rename(work.path() / name, to_plugins / name, error);
    if (error)
      errors.push_back("could not move " + name + " into place: " + error.message());
    else
      copied.push_back(name);
  }
  return copied;
}
