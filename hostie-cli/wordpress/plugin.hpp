#pragma once
#include <crails/cli/command.hpp>
#include <filesystem>

class InstanceEnvironment;

namespace Wordpress
{
  class PluginCommand : public Crails::Command
  {
  public:
    enum ExitCode
    {
      Success = 0,
      BadOptions = 2,
      GlobalPluginLinked = 3, // instance already has same plugin linked to global
      DownloadFailed = 4,
      InvalidPackage = 5,
      NotFound = 6,
      InstallFailed = 7
    };

    std::string_view description() const override
    {
      return "installs or updates a plugin: globally for all the Wordpress instances of a given version, or locally for one instance";
    }

    void options_description(boost::program_options::options_description& options) const override
    {
      using namespace std;
      options.add_options()
        ("name,n", boost::program_options::value<string>(), "plugin slug (eg: akismet)")
        ("url,u", boost::program_options::value<string>(), "plugin download url (defaults to wordpress.org)")
        ("release,r", boost::program_options::value<string>(), "version of the plugin to get from wordpress.org (defaults to the latest)")
        ("version,v", boost::program_options::value<string>(), "wordpress version, for a global plugin (defaults to the current default)")
        ("instance,i", boost::program_options::value<string>(), "install the plugin for this instance only, instead of globally");
    }

    int run() override;

  private:
    std::string download_url() const;
    int install_global(const std::string& slug, const std::string& url);
    int install_local(const std::string& slug, const std::string& url);
    int report(int status, const std::string& slug, const std::string& message, const std::string& version);

    std::string wordpress_version() const;
    std::filesystem::path wordpress_source() const;
    bool link_plugin(const std::filesystem::path& source, const std::string& slug);
    bool link_plugin(const InstanceEnvironment& instance, const std::filesystem::path& source, const std::string& slug) const;
  };
}
