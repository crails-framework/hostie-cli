#pragma once
#include "certificate.hpp"

namespace Nginx
{
  class ConfigureSite : public CertificateCommand
  {
    std::string location_directory_pass(const HttpServer::Location&, bool ssl);
    std::string location_phpfpm_pass(const HttpServer::Location&, bool ssl);
    std::string location_proxy_pass(const HttpServer::Location&, bool ssl);
    std::string location_custom_pass(const HttpServer::Location&, bool ssl);
    std::string location_redirect(const HttpServer::Location&);
    std::string server_locations(bool ssl, bool certified);
    std::string location_https_redirect(const HttpServer::Location&);
    std::string server_common_conf(const std::string_view domain_name);
    std::string server_https_conf(const std::string_view domain_name);
    std::string server_http_conf(const std::string_view domain_name);
    std::string server_conf(const std::string_view domain_name);
    std::string site_conf();
  public:
    void options_description(boost::program_options::options_description& options) const override;
    int run() override;
    int make_site();
  };
}
