#ifndef EXIT_HPP
#define EXIT_HPP

#include <cstdlib>

enum class ExitCode : int {
  SUCCESS = 0,
  INVALID_ARGUMENT_SYNTAX,
  INVALID_ACTION,
  MISSING_REQUIRED_OPTION,
  BAD_OPTION_VALUE,
  FILE_OPEN_FAILED,
};

#define EXIT(code) \
std::exit(static_cast<int>(code))

#endif // EXIT_HPP
