#include "wizard.hpp"
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <filesystem>
#include <boost/process.hpp>
#include <crails/cli/process.hpp>
#include <crails/cli/filesystem.hpp>

using namespace std;
using namespace NextCloud;

static const string_view nextcloud_url =
  "https://download.nextcloud.com/server/releases/latest.tar.bz2";

static filesystem::path find_nextcloud_source()
{
  if (std::getenv("NEXTCLOUD_DIRECTORY") != 0)
    return filesystem::weakly_canonical(getenv("NEXTCLOUD_DIRECTORY"));
  return filesystem::path("/opt/hostie/nextcloud-src");
}

bool Wizard::download_nextcloud()
{
  const filesystem::path target = find_nextcloud_source();

  if (extract_source(nextcloud_url, "nextcloud", target))
  {
    HostieVariables::global->variable("nextcloud-source", target.string());
    HostieVariables::global->save();
    if (generate_version_retriever())
    {
      if (apply_web_permissions(target))
        return true;
      else
        cerr << "Failed to apply proper permissions on " << target << endl;
    }
    else
      cerr << "Failed to generate version-printer.php" << endl;
  }
  return false;
}

bool Wizard::generate_version_retriever()
{
  const filesystem::path target = find_nextcloud_source();
  stringstream stream;

  stream
    << "<?php" << endl
    << "require('./version.php');" << endl
    << "echo implode('.', $OC_Version) . \"\\n\"" << endl;
  return Crails::write_file("nextcloud", target / "version-printer.php", stream.str());
}
