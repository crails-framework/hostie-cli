#include "platforms.hpp"
#include "ubuntu.hpp"
#include "fedora.hpp"

PlatformInstaller nginx_platform_installer()
{
  return PlatformInstaller()
    << ADD_PLATFORM("Ubuntu", "22.04", Nginx::Ubuntu::Wizard)
    << ADD_PLATFORM("Fedora", "40", Nginx::Fedora::Wizard);
}
