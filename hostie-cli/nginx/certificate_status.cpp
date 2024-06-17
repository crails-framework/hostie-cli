#include "certificate_status.hpp"
#include <chrono>
#include <iostream>

using namespace std;
using namespace Nginx;

int CertificateStatus::run()
{
  time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());

  for (const string& domain_name : domain_names())
  {
    cout << "- " << domain_name << ": ";
    if (is_domain_certified(domain_name))
    {
      time_t expiry_at = certificate_expiry_time(domain_name);

      if (expiry_at < now)
        cout << "expired";
      else
      {
        cout << "certified, expires in: "
             << ((expiry_at - now) / 60 / 60 / 24)
             << " days";
      }
    }
    else
    {
      cout << "no certificate";
    }
    cout << endl;
  }
  return 0;
}
