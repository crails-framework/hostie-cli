#include "deploy_command.hpp"
#include "../hostie_variables.hpp"

using namespace Html;
using namespace Crails;
using namespace std;

optional<string> DeployCommand::extract_tarball_from_7z(filesystem::path package_path)
{
  string            expected_tar       = package_path.stem().string();
  filesystem::path  extraction_tmp     = get_extraction_tmp_root() / "7z_tmp";
  filesystem::path  extracted_tar_path = extraction_tmp / expected_tar;
  ExecutableCommand un7z{"7z", {"x", package_path.string(), "-o" + extraction_tmp.string(), "-y"}};

  filesystem::remove_all(extraction_tmp);
  filesystem::create_directories(extraction_tmp);
  cerr << "== " << un7z << endl;
  if (Crails::run_command(un7z))
  {
    if (filesystem::exists(extracted_tar_path))
    {
      return extracted_tar_path.string();
    }
    else
    {
      cerr << "expected tarball `" << expected_tar << "` not found inside 7z archive" << endl;
      filesystem::remove_all(extraction_tmp);
    }
  }
  return {};
}

optional<ExecutableCommand> DeployCommand::make_tarball_extract_command(string package, const filesystem::path& target_dir)
{
  ExecutableCommand untar_command;

  if (package.ends_with(".tar.7z"))
  {
    auto result = extract_tarball_from_7z(filesystem::path(package));
    if (!result.has_value()) return {};
    package = *result;
  }

  if (package.ends_with(".tar"))
  {
    untar_command.path = "tar";
    untar_command.arguments = {"-xf", package, "-C", target_dir.string()};
  }
  else if (package.ends_with(".tar.gz") || package.ends_with(".tgz"))
  {
    untar_command.path = "tar";
    untar_command.arguments = {"-xzf", package, "-C", target_dir.string()};
  }
  else if (package.ends_with(".zip"))
  {
    untar_command.path = "unzip";
    untar_command.arguments = {"-q", package, "-d", target_dir.string()};
  }
  else if (package.ends_with(".7z"))
  {
    untar_command.path = "7z";
    untar_command.arguments = {"x", package, "-o" + target_dir.string(), "-y"};
  }
  else
  {
    cerr << "unknown archive type, extension not handled" << endl;
    return {};
  }
  cerr << "== " << untar_command << endl;
  return untar_command;
}

int DeployCommand::protected_run()
{
  string user      = environment.get_variable("APPLICATION_USER");
  string directory = environment.get_variable("VAR_DIRECTORY");
  string group     = HostieVariables::global->variable("web-group");
  string package   = options["package"].as<string>();
  bool   sync      = options.count("sync") > 0;

  if (filesystem::exists(package))
  {
    // Use a temporary directory for synchronization, or unpack directly otherwise
    filesystem::path target_dir = sync ? (get_extraction_tmp_root() / "sync") : filesystem::path(directory);
    optional<ExecutableCommand> untar_command;

    if (sync)
    {
      filesystem::remove_all(target_dir);
      filesystem::create_directories(target_dir);
    }
    untar_command = make_tarball_extract_command(package, target_dir);
    if (untar_command && Crails::run_command(*untar_command))
    {
      ExecutableCommand chown_command{"chown", {"-R", user + ':' + group, directory}};
      ExecutableCommand chmod_file_command{"find", {directory, "-type", "f", "-exec", "chmod", "640", "{}"}};
      ExecutableCommand chmod_dirs_command{"find", {directory, "-type", "d", "-exec", "chmod", "750", "{}"}};

      if (sync)
      {
        ExecutableCommand sync_cmd{"rsync", {"-a", "--delete", target_dir.string() + "/", directory + "/"}};
        cerr << "== " << sync_cmd << endl;
        if (!Crails::run_command(sync_cmd)) return 2;
      }
      cerr << "== " << chown_command << endl;
      if (!Crails::run_command(chown_command))
        return 2;
      cerr << "== " << chmod_file_command << endl;
      if (!Crails::run_command(chmod_file_command))
        return 2;
      cerr << "== " << chmod_dirs_command << endl;
      if (!Crails::run_command(chmod_dirs_command))
        return 2;
      return 0;
    }
    else
      return 2;
  }
  else
    cerr << "file `" << package << "` does not exist" << endl;
  return 1;
}
