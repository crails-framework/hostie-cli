#pragma once
#include <crails/cli/command_index.hpp>
#include "mysql/platforms.hpp"
#include "postgres/platforms.hpp"
#include "nginx/platforms.hpp"
#include "wordpress/platforms.hpp"

class WizardsIndex : public Crails::CommandIndex
{
public:
  void add_command(std::string_view name, const PlatformInstaller installer)
  {
    Crails::CommandIndex::add_command(name, [installer]()
    {
      return std::make_shared<PlatformInstaller>(installer);
    });
  }

  WizardsIndex()
  {
    using namespace std;
    add_command("mysql", mysql_platform_installer());
    add_command("postgresql", postgres_platform_installer());
    add_command("nginx", nginx_platform_installer());
    add_command("wordpress", wordpress_platform_installer());
  }
};
