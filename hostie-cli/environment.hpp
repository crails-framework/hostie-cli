#pragma once
#include <filesystem>
#include <string>
#include <map>

class InstanceEnvironment
{
public:
  void set_project_name(const std::string& name);
  void load();
  bool save();

  static std::filesystem::path get_root_path();
  const std::string& get_project_name() const { return project_name; }
  const std::filesystem::path& get_path() const { return environment_path; }
  std::string get_variable(const std::string& name) const;
  void set_variable(const std::string& name, const std::string& value);
  void set_variables(const std::map<std::string, std::string>&);
  void append(const std::string_view);

private:
  std::filesystem::path environment_path;
  std::map<std::string, std::string> variables;
  std::string project_name;
};
