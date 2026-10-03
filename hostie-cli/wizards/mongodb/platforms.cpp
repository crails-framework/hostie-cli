#include "platforms.hpp"
#include "ubuntu.hpp"
#include "fedora.hpp"
#include "freebsd.hpp"

PlatformInstaller mongodb_platform_installer()
{
  return PlatformInstaller()
    << ADD_PLATFORM("Ubuntu", "26.04", MongoDB::Ubuntu::Wizard)
    << ADD_PLATFORM("Fedora", "40", MongoDB::Fedora::Wizard)
    << ADD_PLATFORM("FreeBSD", "14", MongoDB::FreeBSD::Wizard);
}
