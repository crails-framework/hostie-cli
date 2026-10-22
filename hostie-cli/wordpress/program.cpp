#include "program.hpp"
#include <cerrno>
#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

extern char** environ;

int Wordpress::run_program(const vector<string>& argv)
{
  vector<char*> arguments;
  posix_spawn_file_actions_t actions;
  pid_t pid;
  int status = 0;

  if (argv.empty())
    return -1;
  for (const string& argument : argv)
    arguments.push_back(const_cast<char*>(argument.c_str()));
  arguments.push_back(nullptr);
  posix_spawn_file_actions_init(&actions);
  posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0);
  if (posix_spawnp(&pid, arguments[0], &actions, nullptr, arguments.data(), environ) != 0)
  {
    posix_spawn_file_actions_destroy(&actions);
    return -1;
  }
  posix_spawn_file_actions_destroy(&actions);
  while (waitpid(pid, &status, 0) < 0)
  {
    if (errno != EINTR)
      return -1;
  }
  return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
