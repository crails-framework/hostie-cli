#pragma once
#include "certificate.hpp"

namespace Nginx
{
  class Certify : public CertificateCommand
  {
  public:
    int run() override;

    bool create_certificate(const std::vector<std::string>&);
  };
}
