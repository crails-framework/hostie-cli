#include "certify.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>

using namespace std;
using namespace Nginx;

int Certify::run()
{
  vector<string> certifiable_domain_names;

  for (const string& domain_name : domain_names())
  {
    if (is_domain_certificate_renewable(domain_name))
      certifiable_domain_names.push_back(domain_name);
  }
  if (create_certificate(certifiable_domain_names))
    return 0;
  else
    cerr << "certbot: failed to certify domains" << endl;
  return -1;
}

bool Certify::create_certificate(const vector<string>& domain_names)
{
  ostringstream command;

  command << "certbot --nginx -n --agree-tos";
  command << " --expand";
  command << " -m " << quoted(webmaster_email());
  for (const string& domain_name : domain_names)
     command << " -d " << quoted(domain_name);
  if (system(command.str().c_str()) == 0)
    return 0;
  return -1;
}
