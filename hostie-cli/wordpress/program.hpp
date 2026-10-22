#pragma once
#include <string>
#include <vector>

namespace Wordpress
{
  // Runs a program (looked up in the PATH) with these arguments, and waits
  // for it. There is no shell in between: each element of `argv` reaches the
  // program as one argument, whatever it contains. The standard output of the
  // program is discarded (what hostie-cli prints on its own standard output
  // is read by other programs), its standard error is left as it is.
  //
  // Returns the exit status, or -1 if the program could not be run or was
  // killed by a signal.
  int run_program(const std::vector<std::string>& argv);
}
