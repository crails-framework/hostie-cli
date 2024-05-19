#include "platforms.hpp"
#include "ubuntu.hpp"
#include "fedora.hpp"

PlatformInstaller wordpress_platform_installer()
{
  return PlatformInstaller()
    << ADD_PLATFORM("Ubuntu", "22.04", Wordpress::Ubuntu::Wizard)
    << ADD_PLATFORM("Fedora", "40", Wordpress::Fedora::Wizard);
}
