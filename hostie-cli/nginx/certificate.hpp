#pragma once
#include "domain_name.hpp"
  
namespace Nginx
{
  class CertificateCommand : public DomainNameCommand
  {
  public:
    static std::filesystem::path certificate_directory(const std::string_view);
    static bool is_domain_certified(const std::string_view);
    static bool is_domain_certificate_up_to_date(const std::string_view);
    static bool is_domain_certificate_renewable(const std::string_view);
    static std::time_t certificate_expiry_time(const std::string_view);
    bool renew_certificates_for(const std::vector<std::string>& domain_names) const;
    bool renew_certificates() const;
  };
}
