#include "plugin_header.hpp"
#include "text_utils.hpp"
#include <crails/utils/semantics.hpp>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

namespace
{
  const size_t header_size = 8192;

  bool iequals_prefix(string_view line, string_view prefix)
  {
    if (line.size() < prefix.size())
      return false;
    for (size_t i = 0 ; i < prefix.size() ; ++i)
    {
      if (tolower(static_cast<unsigned char>(line[i])) != tolower(static_cast<unsigned char>(prefix[i])))
        return false;
    }
    return true;
  }

  // WordPress: /^(?:[ \t]*<\?php)?[ \t\/*#@]*Field:(.*)$/mi, then everything
  // from a closing "*/" or "?>" is dropped.
  optional<string> read_field(string_view contents, string_view field)
  {
    const string key = string(field) + ':';
    size_t line_start = 0;

    while (line_start <= contents.size())
    {
      size_t line_end = contents.find('\n', line_start);
      string_view line = contents.substr(line_start, line_end == string_view::npos ? string_view::npos : line_end - line_start);
      size_t i = 0;

      while (i < line.size() && (line[i] == ' ' || line[i] == '\t'))
        ++i;
      if (line.substr(i, 5) == "<?php")
        i += 5;
      while (i < line.size() && strchr(" \t/*#@", line[i]))
        ++i;
      if (iequals_prefix(line.substr(i), key))
      {
        string value(line.substr(i + key.size()));
        size_t cut = min(value.find("*/"), value.find("?>"));

        if (cut != string::npos)
          value.resize(cut);
        return Crails::trim(value);
      }
      if (line_end == string_view::npos)
        break ;
      line_start = line_end + 1;
    }
    return nullopt;
  }

  string normalized_newlines(string_view contents)
  {
    string result;

    result.reserve(contents.size());
    for (size_t i = 0 ; i < contents.size() ; ++i)
    {
      if (contents[i] == '\r')
      {
        result += '\n';
        if (i + 1 < contents.size() && contents[i + 1] == '\n')
          ++i;
      }
      else
        result += contents[i];
    }
    return result;
  }
}

optional<Wordpress::PluginHeader> Wordpress::parse_plugin_header(string_view raw_contents)
{
  const string contents = normalized_newlines(raw_contents);
  optional<string> name = read_field(contents, "Plugin Name");
  PluginHeader header;

  if (!name || name->empty())
    return nullopt;
  header.name = *name;
  header.version = read_field(contents, "Version").value_or("");
  header.requires_wp = read_field(contents, "Requires at least").value_or("");
  header.requires_php = read_field(contents, "Requires PHP").value_or("");
  return header;
}

optional<Wordpress::PluginHeader> Wordpress::read_plugin_header(const fs::path& file)
{
  ifstream stream(file, ios::binary);
  string contents(header_size, '\0');

  if (!stream.is_open())
    return nullopt;
  stream.read(&contents[0], header_size);
  contents.resize(static_cast<size_t>(stream.gcount()));
  return parse_plugin_header(contents);
}

optional<Wordpress::PluginHeader> Wordpress::find_plugin_header(const fs::path& plugin)
{
  error_code error;

  if (fs::is_regular_file(plugin, error))
    return plugin.extension() == ".php" ? read_plugin_header(plugin) : nullopt;
  if (!fs::is_directory(plugin, error))
    return nullopt;

  const string preferred = plugin.filename().string() + ".php";
  vector<string> candidates;

  for (const auto& entry : fs::directory_iterator(plugin, error))
  {
    const string name = entry.path().filename().string();

    if (entry.path().extension() == ".php" && fs::is_regular_file(entry.path(), error))
      candidates.push_back(name);
  }
  sort(candidates.begin(), candidates.end(), [&preferred](const string& a, const string& b)
  {
    if ((a == preferred) != (b == preferred))
      return a == preferred;
    return a < b;
  });
  for (const string& candidate : candidates)
  {
    if (auto header = read_plugin_header(plugin / candidate))
      return header;
  }
  return nullopt;
}
