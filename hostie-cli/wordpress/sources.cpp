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

bool Wordpress::is_wordpress_version(const string& value)
{
  size_t dots = 0;

  if (value.empty() || value.size() > 16 || value.front() == '.' || value.back() == '.')
    return false;
  for (size_t i = 0 ; i < value.size() ; ++i)
  {
    if (value[i] == '.')
    {
      if (value[i - 1] == '.')
        return false;
      ++dots;
    }
    else if (value[i] < '0' || value[i] > '9')
      return false;
  }
  return dots == 1 || dots == 2;
}

bool Wordpress::is_complete_source(const filesystem::path& source)
{
  error_code error;

  return filesystem::is_directory(source / "wp-includes", error) && filesystem::exists(source / "wp-config.php", error);
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
