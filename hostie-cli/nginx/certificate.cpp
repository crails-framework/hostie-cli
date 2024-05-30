#include "certificate.hpp"
#include <boost/process.hpp>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdlib>

using namespace std;
using namespace Nginx;

static string get_certificate_expiry_date_string(const string_view domain_name)                                  
{ 
  filesystem::path fullchain_path = CertificateCommand::certificate_directory(domain_name) / "fullchain.pem";                             
  stringstream command_stream;                                                                                        
  command_stream
    << "openssl x509 -enddate -noout -in "                                                                            
    << quoted(fullchain_path.string());                                                                               
  boost::process::ipstream stream;
  boost::process::child process(command_stream.str(),                                                                 
    boost::process::std_out > stream                                                                                  
  );
  string output;
  
  if (process.running() && getline(stream, output))                                                                   
  {                                                                                                                   
    std::size_t position = output.find('=');

    if (position != string::npos)                                                                                     
      return output.substr(position + 1);                                                                             
  }
  return string();                                                                                                    
}

filesystem::path CertificateCommand::certificate_directory(const string_view domain_name)
{
  return filesystem::path("/etc/letsencrypt/live") / domain_name;
}

bool CertificateCommand::is_domain_certified(const string_view domain_name)
{
  filesystem::path fullchain_path = certificate_directory(domain_name) / "fullchain.pem";
  filesystem::path privkey_path = certificate_directory(domain_name) / "privkey.pem";

  return filesystem::exists(fullchain_path) && filesystem::exists(privkey_path);
}

bool CertificateCommand::is_domain_certificate_up_to_date(const string_view domain_name)
{
  time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());

  return is_domain_certified(domain_name) && certificate_expiry_time(domain_name) > now;
}

bool CertificateCommand::is_domain_certificate_renewable(const string_view domain_name)
{
  using namespace chrono_literals;
  time_t limit = chrono::system_clock::to_time_t(chrono::system_clock::now() + (30 * 24h));

  return !is_domain_certified(domain_name) || certificate_expiry_time(domain_name) < limit;
}

time_t CertificateCommand::certificate_expiry_time(const string_view domain_name)
{
  string expiry_string = get_certificate_expiry_date_string(domain_name);
  istringstream stream(expiry_string);
  tm time_struct{0};

  stream >> get_time(&time_struct, "%b %d %H:%M:%S %Y");
  return mktime(&time_struct);
}

bool CertificateCommand::renew_certificates() const
{
  vector<string> certifiable_domain_names;
  bool success;

  for (const string& domain_name : domain_names())
  {
    if (is_domain_certificate_renewable(domain_name))
      certifiable_domain_names.push_back(domain_name);
  }
  success = certifiable_domain_names.size() == 0;
  success = success || renew_certificates_for(certifiable_domain_names);
  if (!success)
    cerr << "certbot: failed to certify domains" << endl;
  return success;
}

bool CertificateCommand::renew_certificates_for(const vector<string>& domain_names) const
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
