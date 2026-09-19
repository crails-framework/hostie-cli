#pragma once
#include <filesystem>

struct DirectoryLock
{
  std::filesystem::path path;
public:
  DirectoryLock(const std::filesystem::path& path) : path(path)
  {
    std::filesystem::create_directories(path);
  }

  ~DirectoryLock()
  {
    std::filesystem::remove_all(path);
  }

  operator bool() const
  {
    return std::filesystem::exists(path);
  }
};
