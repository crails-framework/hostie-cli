#include "platforms.hpp"
#include "ubuntu.hpp"
#include "fedora.hpp"

PlatformInstaller odoo_platform_installer()
{
  return PlatformInstaller()
    << ADD_PLATFORM("Ubuntu", "22.04", Odoo::Ubuntu::Wizard)
    << ADD_PLATFORM("Fedora", "40", Odoo::Fedora::Wizard);
}
