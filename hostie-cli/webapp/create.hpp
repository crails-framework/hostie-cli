#pragma once
#include "../standard_creator.hpp"

class Database;

namespace WebApp
{
  struct Deployment;

  class CreateCommand : public StandardCreator
  {
  public:
    enum CustomStateFlag
    {
      DropInCreated = 256
    };

    std::string_view description() const override
    {
      return "creates a new web application instance (deployed afterwards with crails-deploy) with its own system user, database and network port";
    }

    void options_description(boost::program_options::options_description&) const override;
    bool initialize(int argc, const char** argv) override;
    int run() override;

    int cancel(InstanceUser&, Database&);

    virtual bool post_install_actions(const Database&) { return true; }

  private:
    bool install_dropin(const Deployment&);
    void print_next_steps() const;
  };
}
