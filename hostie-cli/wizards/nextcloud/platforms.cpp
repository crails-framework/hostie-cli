#include "platforms.hpp"
#include "ubuntu.hpp"
#include "freebsd.hpp"

PlatformInstaller nextcloud_platform_installer()
{
  return PlatformInstaller()
    << ADD_PLATFORM("Ubuntu", "22.04", NextCloud::Ubuntu::Wizard)
    << ADD_PLATFORM("FreeBSD", "14", NextCloud::FreeBSD::Wizard);
}
