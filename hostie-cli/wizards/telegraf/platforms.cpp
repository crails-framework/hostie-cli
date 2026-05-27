#include "platforms.hpp"
#include "ubuntu.hpp"
#include "freebsd.hpp"

PlatformInstaller telegraf_platform_installer()
{
  return PlatformInstaller()
    << ADD_PLATFORM("Ubuntu", "22.04", UbuntuTelegrafWizard)
    << ADD_PLATFORM("FreeBSD", "14", FreeBSDTelegrafWizard);
}
