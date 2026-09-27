#pragma once
#include <crails/cli/command.hpp>
#include <string_view>
#include <functional>
#include <vector>
#include <iostream>
#include "wizard.hpp"

#define ADD_PLATFORM(provider, release, klass) \
  PlatformInstaller::Installer{ \
    provider, \
    release, \
    []() -> int { return klass().run(); }, \
    []() -> bool { return klass().is_installed(); } \
  }

class PlatformInstaller : public Crails::Command
{
public:
  struct Installer
  {
    const std::string_view distribution;
    const std::string_view version;
    std::function<int ()>  installer;
    std::function<bool ()> is_installed;
  };

  PlatformInstaller& operator<<(Installer runner)
  {
    runners.push_back(runner);
    return *this;
  }

  std::vector<Installer>::const_iterator find_runner() const
  {
    std::vector<std::vector<Installer>::const_iterator> candidates;

    for (auto it = runners.begin() ; it != runners.end() ; ++it)
    {
      const Installer& runner = *it;
      switch (WizardBase::system_matches(runner.distribution, runner.version))
      {
      case 2:
        return it;
      case 1:
        candidates.push_back(it);
      case 0:
        break ;
      }
    }
    if (candidates.size() > 0)
      return *(candidates.begin());
    return runners.end();
  }

  bool is_installed() const
  {
    auto it = find_runner();

    if (it != runners.end())
      return it->is_installed();
    return false;
  }

  int run() override
  {
    auto it = find_runner();

    if (it != runners.end())
      return it->installer();
    std::cerr << "No installer available for your platform" << std::endl;
    return -1;
  }

private:
  std::vector<Installer> runners;
};
