#include "certify.hpp"

using namespace Nginx;

int Certify::run()
{
  return renew_certificates() ? 0 : -1;
}
