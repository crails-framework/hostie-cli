#pragma once
#include "site.hpp"

namespace Nginx
{
  class CertificateStatus : public CertificateCommand
  {
  public:
    int run() override;
  };
}
