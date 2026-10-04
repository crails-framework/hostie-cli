#pragma once
#include <filesystem>
#include <iosfwd>
#include <string>
#include <vector>
#include "../environment.hpp"

namespace WebApp
{
  // Everything hostie-cli knows about the artifacts crails-deploy leaves on the
  // server. The instance name is the contract between the two tools:
  //
  //   unit file          /etc/systemd/system/<name>.service    (crails-deploy)
  //   release env        /usr/share/crails-deploy/<name>       (crails-deploy)
  //   hostie env         <hostie-root>/<name>.env              (hostie-cli)
  //   wiring drop-in     /etc/systemd/system/<name>.service.d/ (hostie-cli)
  //
  // The unit file belongs to crails-deploy: hostie-cli never writes it. The
  // drop-in only re-declares the EnvironmentFile list so that the release
  // environment is read last, and therefore takes precedence.
  struct Deployment
  {
    explicit Deployment(const std::string& name) : name(name) {}

    const std::string name;

    // systemd only: there's no FreeBSD equivalent of a drop-in
    static bool dropin_supported();

    std::filesystem::path unit_path() const;
    std::filesystem::path dropin_directory() const;
    std::filesystem::path dropin_path() const;
    std::filesystem::path release_environment_path() const;

    bool install_dropin(const std::filesystem::path& hostie_environment) const;
    void remove_dropin() const;

    // Removes what crails-deploy created: unit, enablement, drop-in and release
    // environment. Missing files are not an error (instance never deployed).
    void remove_artifacts() const;
  };

  std::vector<std::string> deploy_arguments(const InstanceEnvironment&);
  std::string deploy_arguments_line(const InstanceEnvironment&);
  std::string deploy_example(const InstanceEnvironment&);
  void warn_about_overrides(const InstanceEnvironment&, std::ostream& out);
}
