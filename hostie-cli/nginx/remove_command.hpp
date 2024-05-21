#pragma once
#include "domain_name.hpp"

namespace Nginx
{
  class RemoveCommand : public DomainNameCommand
  {
  public:
    int run() override;
  };

  bool remove_site(const std::string& name);
}
