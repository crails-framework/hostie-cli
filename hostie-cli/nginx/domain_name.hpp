#pragma once
#include <filesystem>
#include "../httpd.hpp"
#include "../instance_command.hpp"

namespace Nginx
{
  class DomainNameCommand : public InstanceCommand
  {
  protected:
    HttpServer::Site site;
  public:
    virtual bool initialize(int argc, const char** argv) override;

    std::string webmaster_email() const;
    std::filesystem::path site_conf_path() const;
  };
}
