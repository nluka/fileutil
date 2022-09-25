#ifndef ACTION_HPP
#define ACTION_HPP

#include <sstream>

#include <boost/program_options.hpp>

#include "exit.hpp"

namespace action {

struct Result {
  std::stringstream m_output;
  ExitCode m_exitCode;

  Result(ExitCode const exitCode) : m_exitCode{exitCode} {}
  Result(ExitCode const exitCode, char const *const msg)
  : m_exitCode{exitCode} {
    m_output << msg;
  }
};

Result repeat(boost::program_options::variables_map const &options);

Result sizerank(boost::program_options::variables_map const &options);

} // namespace action

#endif // ACTION_HPP
