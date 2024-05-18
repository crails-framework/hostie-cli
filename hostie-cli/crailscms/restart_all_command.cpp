#include <crails/cli/terminal.hpp>
#include <iostream>
#include "restart_all_command.hpp"
#include "../service.hpp"

using namespace CrailsCms;
using namespace std;

vector<filesystem::path> RestartAllCommand::collect_environments() const
{
  auto environments = ListCommand::instance_environments();

  environments.erase(
    remove_if(environments.begin(), environments.end(), [](const filesystem::path& filepath)
    {
      return !ListCommand::has_environment_type(filepath, "CrailsCMS");
    })
  );
  return environments;
}

void RestartAllCommand::restart_services(const vector<filesystem::path>& environments) const
{
  for (const filesystem::path& filepath : environments)
  {
    SystemService service;

    service.app_name = filepath.filename().replace_extension().string();
    service.restart();
  }
}

void RestartAllCommand::list_status(const vector<filesystem::path>& environments) const
{
  for (const filesystem::path& filepath : environments)
  {
    SystemService service;
    bool running;

    service.app_name = filepath.filename().replace_extension().string();
    running = service.running();
    cout << "- " << service.app_name << ": ";
    if (running)
      cout << Crails::Terminal::Color::green << "running";
    else
      cout << Crails::Terminal::Color::red << "not running";
    cout << Crails::Terminal::reset_color << endl;
  }
}

int RestartAllCommand::run()
{
  auto environments = collect_environments();

  restart_services(environments);
  sleep(1);
  list_status(environments);
  return 0;
}
