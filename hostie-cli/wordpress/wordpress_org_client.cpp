#include "wordpress_org_client.hpp"
#include "program.hpp"
#include "temporary_directory.hpp"
#include <fstream>
#include <sstream>

using namespace std;
using namespace Wordpress;
namespace fs = std::filesystem;

namespace
{
  const char* const timeout_seconds = "20";

  vector<string> curl_base()
  {
    return {
      "curl", "--fail", "--silent", "--location", "--globoff",
      "--max-time", timeout_seconds, "--proto", "=https", "--proto-redir", "=https",
      "--user-agent", "hostie-cli"
    };
  }

  optional<string> read_file(const fs::path& path)
  {
    ifstream stream(path, ios::binary);
    stringstream contents;

    if (!stream.is_open())
      return nullopt;
    contents << stream.rdbuf();
    return contents.str();
  }

  optional<string> fetch(const string& url)
  {
    TemporaryDirectory directory("/tmp");
    vector<string> command = curl_base();

    if (!directory)
      return nullopt;
    command.insert(command.end(), {"--output", (directory.path() / "answer").string(), url});
    if (run_program(command) != 0)
      return nullopt;
    return read_file(directory.path() / "answer");
  }
}

optional<string> Org::latest_core_version()
{
  if (auto answer = fetch(core_version_url()))
    return parse_latest_core(*answer);
  return nullopt;
}

map<string, Org::PluginRelease> Org::latest_plugin_releases(const vector<string>& slugs)
{
  map<string, PluginRelease> releases;
  TemporaryDirectory directory("/tmp");

  if (!directory || slugs.empty())
    return releases;

  // One curl, all the transfers at once: a request per plugin, one after the
  // other, would make a page of twenty plugins take a while.
  vector<string> command = curl_base();
  command.insert(command.end(), {"--parallel", "--parallel-max", "8"});
  for (size_t i = 0 ; i < slugs.size() ; ++i)
  {
    if (!is_plugin_slug(slugs[i]))
      continue ;
    command.insert(command.end(), {"--output", (directory.path() / to_string(i)).string(), plugin_information_url(slugs[i])});
  }
  // A transfer that fails (an unknown plugin: 404) makes curl exit with an
  // error status, so that is not looked at: the files that exist are what counts.
  // 2 means that this curl is too old to know --parallel.
  if (run_program(command) == 2)
  {
    for (size_t i = 0 ; i < slugs.size() ; ++i)
    {
      vector<string> single = curl_base();

      if (!is_plugin_slug(slugs[i]))
        continue ;
      single.insert(single.end(), {"--output", (directory.path() / to_string(i)).string(), plugin_information_url(slugs[i])});
      run_program(single);
    }
  }
  for (size_t i = 0 ; i < slugs.size() ; ++i)
  {
    if (auto answer = read_file(directory.path() / to_string(i)))
    {
      if (auto release = parse_plugin_release(*answer))
        releases[slugs[i]] = *release;
    }
  }
  return releases;
}

optional<string> Org::search_plugins(const string& query, unsigned int page, unsigned int per_page)
{
  if (auto answer = fetch(plugin_search_url(query, page, per_page)))
    return normalize_search(*answer);
  return nullopt;
}
