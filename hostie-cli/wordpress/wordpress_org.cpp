#include "wordpress_org.hpp"
#include "text_utils.hpp"
#include <crails/datatree.hpp>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace Wordpress;

static const char* const api_base = "https://api.wordpress.org/";

static bool is_alnum_lower(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

// The fields of a plugin that are never shown: asking the API to leave
// them out is the difference between a few KiB and several hundred.
// (The brackets of the parameters are encoded: curl would take them for a range.)
static string without_heavy_fields()
{
  static const char* const fields[] = {
    "description", "sections", "banners", "icons", "reviews", "versions", "contributors",
    "screenshots", "ratings", "tags", "donate_link", "compatibility"
  };
  string query;

  for (const char* field : fields)
    query += string("&request%5Bfields%5D%5B") + field + "%5D=0";
  return query;
}

static void append_member(string& json, bool& first, const char* key, const string& value)
{
  if (value.empty())
    return ;
  ostringstream stream;
  stream << (first ? "" : ",");
  stream << std::quoted(key) << ':' << std::quoted(value);
  json += stream.str();
  first = false;
}

static void append_number(string& json, bool& first, const char* key, unsigned long value)
{
  ostringstream stream;
  stream << (first ? "" : ",");
  stream << std::quoted(key) << ':' << to_string(value);
  json += stream.str();
  first = false;
}

bool Org::is_plugin_slug(string_view value)
{
  if (value.empty() || value.size() > 64 || !is_alnum_lower(value.front()))
    return false;
  for (char c : value)
  {
    if (!is_alnum_lower(c) && c != '-' && c != '_')
      return false;
  }
  return true;
}

bool Org::is_plugin_release(string_view value)
{
  if (value.empty() || value.size() > 32 || !isalnum(static_cast<unsigned char>(value.front())))
    return false;
  for (char c : value)
  {
    if (!isalnum(static_cast<unsigned char>(c)) && c != '.' && c != '-' && c != '_')
      return false;
  }
  return true;
}

string Org::core_version_url()
{
  return string(api_base) + "core/version-check/1.7/";
}

string Org::plugin_information_url(string_view slug)
{
  return string(api_base) + "plugins/info/1.2/?action=plugin_information&request%5Bslug%5D=" + url_encode(slug) + without_heavy_fields();
}

string Org::plugin_search_url(string_view query, unsigned int page, unsigned int per_page)
{
  return string(api_base) + "plugins/info/1.2/?action=query_plugins"
    + "&request%5Bsearch%5D=" + url_encode(query)
    + "&request%5Bpage%5D=" + to_string(page)
    + "&request%5Bper_page%5D=" + to_string(per_page)
    + without_heavy_fields();
}

string Org::plugin_download_url(string_view slug, string_view release)
{
  string url = "https://downloads.wordpress.org/plugin/" + url_encode(slug);

  if (!release.empty())
    url += '.' + url_encode(release);
  return url + ".zip";
}

optional<string> Org::parse_latest_core(string_view json)
{
  DataTree root;
  try { root.from_json(string(json)); } catch (...) { }
  Data offers = root["offers"];

  if (offers.exists() && offers.is_array() && offers.count() > 0)
  {
    Data offer = *offers.begin();
    string version = offer.defaults_to<string>("current");

    if (version.empty())
      version = offer.defaults_to<string>("version");
    if (!version.empty())
      return version;
  }
  return nullopt;
}

optional<Org::PluginRelease> Org::parse_plugin_release(string_view json)
{
  DataTree root;
  try { root.from_json(string(json)); } catch (...) { }
  if (!root["error"].exists())
  {
    PluginRelease release;

    release.version = root["version"].defaults_to<string>("");
    release.requires_wp = root["requires"].defaults_to<string>("");
    if (!release.version.empty())
      return release;
  }
  return nullopt;
}

optional<string> Org::normalize_search(string_view json)
{
  DataTree root;

  try { root.from_json(string(json)); } catch (...) { }
  if (root["error"].exists())
    return nullopt;

  Data list = root["plugins"];
  Data info = root["info"];

  if (!list.exists() || !info.exists())
    return nullopt;

  string result = "{\"page\":" + to_string(info["page"].defaults_to<unsigned int>(1))
         + ",\"pages\":" + to_string(info["pages"].defaults_to<unsigned int>(1))
         + ",\"total\":" + to_string(info["results"].defaults_to<unsigned int>(0))
         + ",\"plugins\":[";

  bool first_plugin = true;

  // Data's iterator handles both proper JSON arrays and PHP-holed object representations automatically
  for (const Data& plugin : list)
  {
    const string slug = plugin["slug"].defaults_to<string>("");
    bool first = true;

    if (!Org::is_plugin_slug(slug))
      continue ;
    result += (first_plugin ? "{" : ",{");
    first_plugin = false;
    append_member(result, first, "slug", slug);
    append_member(result, first, "name", decode_html_entities(plugin["name"].defaults_to<string>("")));
    append_member(result, first, "version", plugin["version"].defaults_to<string>(""));
    append_member(result, first, "short_description", decode_html_entities(plugin["short_description"].defaults_to<string>("")));
    append_number(result, first, "active_installs", plugin["active_installs"].defaults_to<unsigned long>(0));
    append_number(result, first, "rating", plugin["rating"].defaults_to<unsigned long>(0));
    append_member(result, first, "requires", plugin["requires"].defaults_to<string>(""));
    append_member(result, first, "requires_php", plugin["requires_php"].defaults_to<string>(""));
    append_member(result, first, "tested", plugin["tested"].defaults_to<string>(""));
    append_member(result, first, "last_updated", plugin["last_updated"].defaults_to<string>(""));
    result += '}';
  }
  return result + "]}";
}
