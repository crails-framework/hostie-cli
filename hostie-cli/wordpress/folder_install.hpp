#pragma once
#include <filesystem>

struct InstanceUser;

namespace Wordpress
{
  struct FolderInstaller
  {
    enum Action { Install, Uninstall };

    const InstanceUser& user;
    const std::filesystem::path var_directory;
    const std::filesystem::path wordpress_source;
    Action action;

    std::filesystem::path wp_content_source() const;
    std::filesystem::path wp_content_target() const;
    std::string web_group() const;

    bool operator()() const;
    bool install();
    bool uninstall();
  private:
    void linking_root_directories() const;
    void create_wp_content() const;
    void create_uploads() const;

    void create_directories(const std::filesystem::path&) const;
    void create_directory_symlink(const std::filesystem::path& source, const std::filesystem::path& target) const;
    void create_hard_link(const std::filesystem::path& source, const std::filesystem::path& target) const;
    void set_owners(const std::filesystem::path& target, const std::string& user, const std::string& group) const;
  };
}
