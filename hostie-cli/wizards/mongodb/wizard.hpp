#pragma once
#include "../wizard.hpp"
#include <cstdlib>

namespace MongoDB
{
  class Wizard : public WizardBase
  {
    HostieVariables& store;
  public:
    static constexpr const char* default_mongodb_version = "9.0";

    Wizard() : store(*HostieVariables::global) {}

    bool is_installed() const { return store.has_variable("mongodb"); }
    int run()
    {
      store.variable("mongodb", "1");
      store.save();
      return 0;
    }

    std::string get_mongodb_version() const
    {
      const char* value = std::getenv("MONGODB_VERSION");
      return value ? value : default_mongodb_version;
    }
  };
}
