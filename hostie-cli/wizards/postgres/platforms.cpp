#include "platforms.hpp"
#include "ubuntu.hpp"
#include "fedora.hpp"
#include "freebsd.hpp"

PlatformInstaller postgres_platform_installer()
{
  return PlatformInstaller()
    << ADD_PLATFORM("Ubuntu", "22.04", UbuntuPostgresWizard)
    << ADD_PLATFORM("Fedora", "40", FedoraPostgresWizard)
    << ADD_PLATFORM("FreeBSD", "14", FreeBSDPostgresWizard);
}
