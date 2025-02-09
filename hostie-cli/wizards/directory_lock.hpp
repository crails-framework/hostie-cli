#pragma once
#include <filesystem>

struct DirectoryLock
{
  std::filesystem::path path;
public:
  DirectoryLock(const filesystem::path& path) : path(path)
  {
    filesystem::create_directories(path);
  }

  ~DirectoryLock()
  {
    filesystem::remove_all(path);
  }

  operator bool() const
  {
    return filesystem::exists(path);
  }
};
