#include "wizard.hpp"

using namespace std;
using namespace Nginx;

int Wizard::run()
{
  store.variable("httpd", "nginx");
  store.save();
  return true;
}
