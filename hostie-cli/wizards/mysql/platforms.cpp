#include "platforms.hpp"
#include "ubuntu.hpp"
#include "fedora.hpp"

PlatformInstaller mysql_platform_installer()
{
  return PlatformInstaller()
    << ADD_PLATFORM("Ubuntu", "22.04", UbuntuMysqlWizard)
    << ADD_PLATFORM("Fedora", "40", FedoraMysqlWizard);
}
