#pragma once
#include <crails/cli/command.hpp>
#include <filesystem>

class InstanceEnvironment;

namespace Wordpress
{
  class PluginCommand : public Crails::Command
  {
    std::filesystem::path wp_folder;
  public:
    std::string_view description() const override
    {
      return "installs or updates a plugin globally for all the Wordpress instances of a given version";
    }

    void options_description(boost::program_options::options_description& options) const override
    {
      using namespace std;
      options.add_options()
        ("name,n", boost::program_options::value<string>(), "plugin name")
        ("url,u", boost::program_options::value<string>(), "plugin download url")
        ("version,v", boost::program_options::value<string>(), "wordpress version (defaults to the current default)");
    }

    int run() override;

  private:
    std::string wordpress_version() const;
    std::filesystem::path wordpress_source() const;
    std::filesystem::path plugin_path() const;
    bool pull_plugin(const std::string& url);
    bool link_plugin();
    bool link_plugin(const InstanceEnvironment& instance, const std::filesystem::path& source) const;
  };
}
