#pragma once
#include <filesystem>
#include <string>

struct SystemService
{
  std::string app_name, app_user, app_group;
  std::string start_command, stop_command;
  std::string kill_mode;
  std::filesystem::path runtime_directory;
  std::filesystem::path environment_path;

  std::filesystem::path service_file_path() const;
  bool start();
  bool stop();
  bool restart();
  bool running();
  bool status(); // TO REMOVE
  bool require();
  bool service_file_exists() const;
  bool reload_service_files() const;
  bool create_service_file() const;
  std::string service_file_contents() const;
};
