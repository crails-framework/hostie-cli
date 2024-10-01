#include "wizard.hpp"
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <filesystem>
#include <boost/process.hpp>
#include <crails/cli/with_path.hpp>
#include <crails/cli/process.hpp>
#include <crails/cli/filesystem.hpp>
#include <crails/read_file.hpp>
#include <crails/utils/split.hpp>

using namespace std;
using namespace Wordpress;

static const string wp_config_src = 
  "<?php require_once($_SERVER[\"App-Root\"] . \"/wp-config.php\"); ?>";

static string find_latest_version(const WizardBase& wizard)
{
  const string_view error_message = "failed to determine latest version of Wordpress";
  const string_view version_url = "https://api.wordpress.org/core/version-check/1.7/";
  const filesystem::path version_file = "/tmp/wordpress.versions.json";

  if (Crails::require_command("curl"))
  {
    if (wizard.download_file(version_url, version_file))
    {
      const string pattern("\"current\":\"");
      string json, version;
      size_t start, end;

      Crails::read_file(version_file.string(), json);
      start = json.find(pattern) + pattern.length();
      end = json.find('"', start);
      version = json.substr(start, end - start);
      cout << "Latest version of wordpress: `" << version << '`' << endl;
      return version;
    }
    else
      cerr << "failed to download `" << version_url << "`, " << error_message << endl;
  }
  else
    cerr << "curl not installed, " << error_message << endl;
  return "6.6.2";
}

static string find_wordpress_version(const WizardBase& wizard)
{
  const char* from_env = std::getenv("WORDPRESS_VERSION");

  return from_env ? string(from_env) : find_latest_version(wizard);
}

static string wordpress_url_for(const string& version)
{
  return "https://wordpress.org/wordpress-"
        + version + ".tar.gz";
}

static string wordpress_language_url_for(const string& language, const string& version)
{
  return "https://downloads.wordpress.org/translations/core/"
        + version + '/' + language + ".zip";
}

static void download_language(const WizardBase& wizard, const filesystem::path& target, const string& language, const string& version)
{
  Crails::WithPath path(target / "wp-content" / "languages");

  wizard.download_zip_archive(
    wordpress_language_url_for(language, version)
  );
}

static vector<string> wordpress_languages()
{
  const string variable = HostieVariables::global->variable_or("wordpress-languages", "");

  return Crails::split<string, vector<string>>(variable, ';');
}

filesystem::path find_wordpress_source(const string& version)
{
  if (std::getenv("WORDPRESS_DIRECTORY") != 0)
    return filesystem::weakly_canonical(getenv("WORDPRESS_DIRECTORY")) / version;
  return filesystem::path("/opt/hostie/wordpress-src") / version;
}

bool Wizard::download_wordpress()
{
  const string version = find_wordpress_version(*this);
  const string wordpress_url = wordpress_url_for(version);
  const filesystem::path target = find_wordpress_source(version);

  if (extract_source(wordpress_url, "wordpress", target))
  {
    for (const string& language : wordpress_languages())
      download_language(*this, target, language, version);
    if (Crails::write_file("wordpress-installer", (target / "wp-config.php").string(), wp_config_src))
    {
      HostieVariables::global->variable("wordpress-version", version);
      HostieVariables::global->variable("wordpress-source", target.string());
      HostieVariables::global->save();
      if (apply_web_permissions(target))
        return true;
      else
        cerr << "Failed to apply proper permissions on " << target << endl;
    }
    else
      cerr << "Failed to overwrite wp-config.php" << endl;
  }
  return false;
}
