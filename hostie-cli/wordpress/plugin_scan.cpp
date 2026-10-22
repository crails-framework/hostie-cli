#include "plugin_scan.hpp"
#include "plugin_header.hpp"
#include <algorithm>
#include <map>
#include <set>
#include <sys/stat.h>

using namespace std;
namespace fs = std::filesystem;
using Wordpress::PluginEntry;

namespace
{
  bool is_ignored(const string& filename)
  {
    return filename.empty() || filename[0] == '.' || filename == "index.php";
  }

  string slug_of(const fs::path& path)
  {
    return path.extension() == ".php" && !fs::is_directory(path) ? path.stem().string() : path.filename().string();
  }

  void apply_header(PluginEntry& entry, const fs::path& path)
  {
    if (auto header = Wordpress::find_plugin_header(path))
    {
      entry.name = header->name;
      entry.version = header->version;
    }
  }

  // Where a symbolic link leads, without needing the target to exist
  fs::path link_target(const fs::path& link)
  {
    error_code error;
    fs::path target = fs::read_symlink(link, error);

    if (error)
      return fs::path();
    if (target.is_relative())
      target = link.parent_path() / target;
    return target.lexically_normal();
  }

  bool points_into(const fs::path& target, const fs::path& source_plugins)
  {
    error_code error;
    const fs::path canonical_root = fs::weakly_canonical(source_plugins, error);

    return Wordpress::is_inside(target, source_plugins.lexically_normal())
        || (!error && Wordpress::is_inside(target, canonical_root));
  }

  // A file that is a hard link of the file of the same name in the source
  bool is_hard_link_of_source(const fs::path& path, const fs::path& source_plugins)
  {
    struct stat st;
    error_code error;
    const fs::path counterpart = source_plugins / path.filename();

    return lstat(path.c_str(), &st) == 0 && st.st_nlink > 1
        && fs::exists(counterpart, error) && fs::equivalent(path, counterpart, error);
  }

  void scan_instance(const fs::path& instance_plugins, const fs::path& source_plugins, vector<PluginEntry>& entries)
  {
    error_code error;

    if (!fs::is_directory(instance_plugins, error))
      return ;
    for (const auto& item : fs::directory_iterator(instance_plugins, error))
    {
      const fs::path path = item.path();
      const string filename = path.filename().string();
      const fs::file_status status = fs::symlink_status(path, error);
      PluginEntry entry;

      if (is_ignored(filename))
        continue ;
      entry.slug = slug_of(path);
      if (fs::is_symlink(status))
      {
        const fs::path target = link_target(path);

        entry.scope = points_into(target, source_plugins) ? PluginEntry::Global : PluginEntry::Local;
        entry.broken = !fs::exists(path, error);
        if (!entry.broken)
          apply_header(entry, path);
      }
      else if (fs::is_directory(status))
      {
        entry.scope = PluginEntry::Local;
        entry.overrides_global = fs::exists(source_plugins / filename, error);
        apply_header(entry, path);
      }
      else if (fs::is_regular_file(status) && path.extension() == ".php")
      {
        entry.scope = is_hard_link_of_source(path, source_plugins) ? PluginEntry::Global : PluginEntry::Local;
        entry.overrides_global = entry.scope == PluginEntry::Local && fs::exists(source_plugins / filename, error);
        apply_header(entry, path);
        // Any other PHP file of that directory is not a plugin
        if (entry.name.empty())
          continue ;
      }
      else
        continue ;
      entries.push_back(entry);
    }
  }

  void scan_source(const fs::path& source_plugins, vector<PluginEntry>& entries)
  {
    error_code error;
    set<string> known;

    if (!fs::is_directory(source_plugins, error))
      return ;
    for (const PluginEntry& entry : entries)
      known.insert(entry.slug);
    for (const auto& item : fs::directory_iterator(source_plugins, error))
    {
      const fs::path path = item.path();
      PluginEntry entry;

      if (is_ignored(path.filename().string()))
        continue ;
      entry.slug = slug_of(path);
      if (known.count(entry.slug) || !(fs::is_directory(path, error) || path.extension() == ".php"))
        continue ;
      entry.scope = PluginEntry::Global;
      entry.linked = false;
      apply_header(entry, path);
      if (!fs::is_directory(path, error) && entry.name.empty())
        continue ;
      entries.push_back(entry);
    }
  }
}

bool Wordpress::is_inside(const fs::path& path, const fs::path& root)
{
  fs::path normalized_root = root.lexically_normal();
  fs::path normalized_path = path.lexically_normal();

  // "a/b/" is "a/b" and an empty file name: not a component to compare
  if (!normalized_root.empty() && normalized_root.filename().empty())
    normalized_root = normalized_root.parent_path();
  if (normalized_root.empty() || normalized_path.empty())
    return false;
  auto mismatch_point = std::mismatch(normalized_root.begin(), normalized_root.end(), normalized_path.begin(), normalized_path.end());
  return mismatch_point.first == normalized_root.end();
}

vector<PluginEntry> Wordpress::scan_plugins(const fs::path& instance_plugins, const fs::path& source_plugins)
{
  vector<PluginEntry> entries;

  scan_instance(instance_plugins, source_plugins, entries);
  scan_source(source_plugins, entries);
  sort(entries.begin(), entries.end(), [](const PluginEntry& a, const PluginEntry& b) { return a.slug < b.slug; });
  return entries;
}
