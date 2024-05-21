#pragma once
#include <string>
#include <filesystem>
#include <vector>
#include <map>
#include <functional>

class InstanceEnvironment;

namespace HttpServer
{
  class Site;

  typedef std::function<void(const InstanceEnvironment&, Site&)> \
    SiteInitializer;

  enum LocationType
  {
    DirectoryLocation,
    AppProxyLocation,
    PhpFpmLocation,
  };

  enum SslState
  {
    NoSslState = 0,
    SslOmit = 1,
    SslRequired = 2
  };

  struct Location
  {
    std::string path = "/";
    std::string target;
    LocationType type = AppProxyLocation;
    int ssl = NoSslState;
    std::vector<std::string> custom_settings;
  };

  struct Upstream
  {
    std::string name;
    std::string ip;
    unsigned short port;
  };

  struct Site
  {
    std::string name;
    std::filesystem::path var_directory;
    std::vector<Upstream> upstreams;
    std::vector<Location> locations;
    std::vector<std::string> custom_settings;
    static const std::map<std::string, SiteInitializer> initializers;
  };
}
