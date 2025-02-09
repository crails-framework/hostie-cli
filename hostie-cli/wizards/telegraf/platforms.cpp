#include "platforms.hpp"
#include "ubuntu.hpp"

PlatformInstaller telegraf_platform_installer()
{
  return PlatformInstaller()
    << ADD_PLATFORM("Ubuntu", "22.04", UbuntuTelegrafWizard);
}
