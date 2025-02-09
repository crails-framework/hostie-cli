#pragma once
#include "wizard.hpp"
#include "../ubuntu.hpp"
#include "../directory_lock.hpp"

class UbuntuTelegrafWizard : public TelegrafWizard<UbuntuWizard>
{
public:
  UbuntuTelegrafWizard()
  {
    requirements.push_back("telegraf");
  }

  bool start_service() override
  {
    return Crails::run_command("systemctl enable telegraf.service")
        && Crails::run_command("systemctl start telegraf.service");
  }

  bool add_telegraf_repository() override
  {
    using namespace std;
    const filesystem::path tmp_path("/tmp/hostie-wizard-influx");
    DirectoryLock tmp_dir(tmp_path);
    const filesystem::path key_path = tmp_path / "archive.key";
    const filesystem::path gpg_path = tmp_path / "archive.gpg";
    const string_view archive_key_url("https://repos.influxdata.com/influxdata-archive.key");
    const string_view archive_key_sum("943666881a1b8d9b849b74caebf02d3465d6beb716510d86a39f6c8e8dac7515");

    if (tmp_dir)
    {
      if (download_file(archive_key_url, key_path) && checksum_sha256(key_path, archive_key_sum))
      {
        return gpg_dearmor(key_path, gpg_path)
            && add_repository("influxdata", "https://repos.influxdata.com/debian stable main", gpg_path);
      }
      else
        cerr << "failed to download influxdata-archive.key from " << quoted(archive_key_url) << endl;
    }
    else
      cerr << "could not create directory" << tmp_path << endl;
    return false;
  }
};
