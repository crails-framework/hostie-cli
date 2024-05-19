#pragma once
#include <crails/cli/command.hpp>
#include <string_view>
#include <functional>
#include <vector>
#include <iostream>
#include "wizard.hpp"

#define ADD_PLATFORM(provider, release, klass) \
  PlatformInstaller::Installer{provider, release, []() -> int { return klass().run(); }}

class PlatformInstaller : public Crails::Command
{
public:
  struct Installer
  {
    const std::string_view distribution;
    const std::string_view version;
    std::function<int ()>  installer;
  };

  PlatformInstaller& operator<<(Installer runner)
  {
    runners.push_back(runner);
    return *this;
  }

  int run() override
  {
    std::vector<Installer> candidates;

    for (const Installer runner : runners)
    {
      switch (WizardBase::system_matches(runner.distribution, runner.version))
      {
      case 2:
        return runner.installer();
      case 1:
        candidates.push_back(runner);
      case 0:
        break ;
      }
    }
    if (candidates.size() > 0)
      return candidates.begin()->installer();
    std::cerr << "No installer available for your platform" << std::endl;
    return -1;
  }

private:
  std::vector<Installer> runners;
};
