#pragma once
#include "../live_instance_command.hpp"
#include <crails/cli/process.hpp>

namespace Html
{
  class DeployCommand : public LiveInstanceCommand
  {
  public:
    std::string_view application_type() const override
    {
      return "HTML";
    }

    std::string_view description() const override
    {
      return "deploys a tar archive as the HTML site";
    }
  
    void options_description(boost::program_options::options_description& options) const override
    {
      using namespace std;
      options.add_options()
        ("package,p", boost::program_options::value<string>(), "absolute path of the tarball to extract")
        ("sync,s", "synchronize directory by deleting files not present in the archive");
    }

    std::filesystem::path get_extraction_tmp_root() const
    {
      std::string directory = environment.get_variable("VAR_DIRECTORY");
      return "/var/tmp/hostie-cli/" + directory;
    }

    int run() override
    {
      int status = protected_run();

      std::filesystem::remove_all(get_extraction_tmp_root());
      return status;
    }
  protected:
    int protected_run();
    std::optional<Crails::ExecutableCommand> make_tarball_extract_command(std::string package, const std::filesystem::path& target_dir);
    std::optional<std::string> extract_tarball_from_7z(std::filesystem::path package_path);
  };
}
