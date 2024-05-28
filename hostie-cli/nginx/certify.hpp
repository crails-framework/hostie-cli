#pragma once
#include "certificate.hpp"

namespace Nginx
{
  class Certify : public CertificateCommand
  {
  public:
    int run() override;
  };
}
