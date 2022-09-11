#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "action.hpp"
#include "exit.hpp"
#include "program-options.hpp"

namespace bpo = boost::program_options;

void print_help_msg() {
  std::cout <<
    "ACTIONS: repeat\n"
    "repeat:\n"
    "  duplicates a file's content N times, writing it as a new file.\n"
    "  REQUIRED: --in,    -i | pathname of file to repeat\n"
    "  REQUIRED: --times, -t | number to times to repeat content\n"
    "  REQUIRED: --out,   -n | pathname of resultant file\n"
  ;
}

int main(int const argc, char const *const *const argv) {
  if (argc == 1) {
    print_help_msg();
    EXIT(ExitCode::SUCCESS);
  }

  bpo::variables_map options;
  try {
    bpo::store(bpo::parse_command_line(argc, argv, prgopt::options_descrip()), options);
  } catch (std::exception const &err) {
    std::cerr << "fatal: " << err.what() << '\n';
    EXIT(ExitCode::INVALID_ARGUMENT_SYNTAX);
  }
  bpo::notify(options);

  auto const result = [argv, &options]() {
    std::string const action = argv[1];
    if (action == "repeat") {
      return action::repeat(options);
    } else {
      action::Result res(ExitCode::INVALID_ACTION);
      res.m_output  << "fatal: `" << action << "` is not a valid action";
      return res;
    }
  }();

  std::cout << result.m_output.str() << '\n';
  EXIT(result.m_exitCode);
}
