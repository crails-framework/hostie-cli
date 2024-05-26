#include "platforms.hpp"
#include "ubuntu.hpp"

PlatformInstaller nextcloud_platform_installer()
{
  return PlatformInstaller()
    << ADD_PLATFORM("Ubuntu", "22.04", NextCloud::Ubuntu::Wizard);
}
