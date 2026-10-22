#include "sources.hpp"
#include <algorithm>
#include <cstdlib>

using namespace std;

filesystem::path Wordpress::source_root()
{
  const char* from_env = std::getenv("WORDPRESS_DIRECTORY");

  if (from_env != nullptr)
    return filesystem::weakly_canonical(from_env);
  return filesystem::path("/opt/hostie/wordpress-src");
}

vector<string> Wordpress::installed_versions(const filesystem::path& root)
{
  vector<string> versions;
  error_code error;

  if (filesystem::is_directory(root, error))
  {
    for (const auto& entry : filesystem::directory_iterator(root, error))
    {
      if (filesystem::is_directory(entry.path() / "wp-includes", error))
        versions.push_back(entry.path().filename().string());
    }
  }
  sort(versions.begin(), versions.end());
  return versions;
}
