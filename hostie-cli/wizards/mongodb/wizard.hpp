#pragma once
#include "../wizard.hpp"

namespace MongoDB
{
  class Wizard : public WizardBase
  {
    HostieVariables& store;
    std::string password;
  public:
    static constexpr const char* default_mongodb_version = "9.0";

    Wizard() : store(*HostieVariables::global) {}

    bool is_installed() const { return store.has_variable("mongodb"); }

    int  run();
    bool prepare_root_user() const;
    bool enable_authorization();
    bool authenticate_root() const;

    virtual bool restart_service() = 0;

    std::string get_mongodb_version() const
    {
      const char* value = std::getenv("MONGODB_VERSION");
      return value ? value : default_mongodb_version;
    }

    virtual std::string get_mongod_conf_path() const
    {
      return "/etc/mongod.conf";
    }

    std::string connection_string() const
    {
      return "mongodb://127.0.0.1:27017/admin";
    }
  };
}
