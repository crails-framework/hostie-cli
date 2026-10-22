#pragma once
#include <filesystem>

namespace Wordpress
{
  // A directory only root can enter, with a name nobody can guess (mkdtemp),
  // removed with everything in it at the end of its life.
  //
  // It is created in `parent` rather than in /tmp: what is built in there can
  // then be moved into place with rename(), which does not work across
  // filesystems (and /tmp very often is one of its own).
  class TemporaryDirectory
  {
  public:
    explicit TemporaryDirectory(const std::filesystem::path& parent);
    ~TemporaryDirectory();
    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    explicit operator bool() const { return !path_.empty(); }
    const std::filesystem::path& path() const { return path_; }

  private:
    std::filesystem::path path_;
  };
}
