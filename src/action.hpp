#ifndef ACTION_HPP
#define ACTION_HPP

#include <string>

#include <boost/program_options.hpp>

namespace action {
  std::string repeat_perform(int argc, char const* const* argv);
  std::string repeat_help_msg();
  boost::program_options::options_description repeat_options_desc();

  std::string sizerank_perform(int argc, char const* const* argv);
  std::string sizerank_help_msg();
  boost::program_options::options_description sizerank_options_desc();
}

#endif // ACTION_HPP