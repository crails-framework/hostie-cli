#include "folder_install.hpp"
#include "../user.hpp"
#include "../hostie_variables.hpp"
#include "../file_ownership.hpp"
#include <iostream>
#include <vector>

using namespace std;
using namespace Wordpress;

filesystem::path FolderInstaller::wp_content_source() const
{
  return wordpress_source / "wp-content";
}

filesystem::path FolderInstaller::wp_content_target() const
{
  return var_directory / "wp-content";
}

string FolderInstaller::web_group() const
{
  return HostieVariables::global->variable_or("web-group", "www-data");
}

void FolderInstaller::create_directories(const filesystem::path& path) const
{
  if (action == Install)
    filesystem::create_directories(path);
}

void FolderInstaller::create_directory_symlink(const filesystem::path& source, const filesystem::path& target) const
{
  if (action == Install)
    filesystem::create_directory_symlink(source, target);
  else
    filesystem::remove(target);
}

void FolderInstaller::create_hard_link(const filesystem::path& source, const filesystem::path& target) const
{
  if (action == Install)
    filesystem::create_hard_link(source, target);
  else
    filesystem::remove(target);
}

void FolderInstaller::set_owners(const filesystem::path& target, const string& user, const string& group) const
{
  if (action == Install)
  {
    Crails::chown(target, user);
    Crails::chgrp(target, group);
  }
}

void FolderInstaller::linking_root_directories() const
{
  for (const auto& entry : filesystem::directory_iterator(wordpress_source))
  {
    filesystem::path path = entry.path();

    if (filesystem::is_directory(path))
    {
      if (filesystem::equivalent(path, wp_content_source())) continue;
      create_directory_symlink(path, var_directory / path.filename());
    }
    else if (path.filename() != "wp-config.php")
      create_hard_link(path, var_directory / path.filename());
  }
}

void FolderInstaller::create_wp_content() const
{
  create_directories(wp_content_target());
  create_hard_link        (wp_content_source() / "index.php",  wp_content_target() / "index.php");
  create_directory_symlink(wp_content_source() / "languages",  wp_content_target() / "languages");
  create_directory_symlink(wp_content_source() / "mu-plugins", wp_content_target() / "mu-plugins");
  for (const string& name : vector<string>{"plugins", "themes"})
  {
    const filesystem::path source_folder = wp_content_source() / name;
    const filesystem::path target_folder = wp_content_target() / name;

    create_directories(target_folder);
    set_owners(target_folder, user.name, web_group());
    for (const auto& entry : filesystem::directory_iterator(source_folder))
    {
      filesystem::path path = entry.path();
      filesystem::path target = target_folder / path.filename();

      if (action == Install && filesystem::exists(target))
        continue ;
      else if (filesystem::is_directory(path))
        create_directory_symlink(path, target);
      else
        create_hard_link(path, target);
    }
  }
}

void FolderInstaller::create_uploads() const
{
  const filesystem::path upload_folder = wp_content_target() / "uploads";

  if (!filesystem::exists(upload_folder))
  {
    create_directories(upload_folder);
    set_owners(upload_folder, user.name, web_group());
  }
}

bool FolderInstaller::operator()() const
{
  if (!wordpress_source.empty())
  {
    linking_root_directories();
    create_wp_content();
    create_uploads();
    return true;
  }
  else
    cerr << "wordpress source not found" << endl;
  return false;
}

bool FolderInstaller::install()
{
  action = Install;
  return operator()();
}

bool FolderInstaller::uninstall()
{
  action = Uninstall;
  return operator()();
}
