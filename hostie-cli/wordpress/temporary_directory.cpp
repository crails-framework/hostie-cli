#include "temporary_directory.hpp"
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>

using namespace std;
namespace fs = std::filesystem;

Wordpress::TemporaryDirectory::TemporaryDirectory(const fs::path& parent)
{
  string pattern = (parent / ".hostie-XXXXXX").string();
  vector<char> buffer(pattern.begin(), pattern.end());

  buffer.push_back('\0');
  if (mkdtemp(buffer.data()) != nullptr)
    path_ = buffer.data();
}

Wordpress::TemporaryDirectory::~TemporaryDirectory()
{
  error_code error;

  if (!path_.empty())
    fs::remove_all(path_, error);
}
