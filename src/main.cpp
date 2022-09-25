#include <iostream>
#include <string>

#include "action.hpp"
#include "exit.hpp"
#include "program-options.hpp"

using namespace std;
namespace bpo = boost::program_options;

void print_help_msg() {
  cout <<
    "USAGE: <action> [option]...\n"
    "---------------\n"
    "VERSION: 1.0.0\n"
    "SOURCE: https://github.com/nluka/fileutil\n"
    "---------------\n"
    "AVAILABLE ACTIONS: repeat, sizerank\n"
    "---------------\n"
    "repeat:\n"
    "  duplicates a file's content N times, writing it as a new file.\n"
    "  --in    * | pathname of file to repeat\n"
    "  --count * | number to times to duplicate content (1 = make copy)\n"
    "  --out   * | pathname of resultant file\n"
    "          * = required\n"
    "---------------\n"
    "sizerank:\n"
    "  ranks top N largest files in directory/subdirectories.\n"
    "  --dir               | directory to search [default: cwd]\n"
    "  --recursive         | enables recusive iteration through all subdirectories [default: off]\n"
    "  --top               | how many top entries to rank [default: 10]\n"
    // "  --type      | smallest|sm|largest|lg [default: largest]\n"
    // "  --order     | ascending|asc|descending|desc [default: descending]\n"
    "  --minsize           | smallest file size (in bytes) to consider [default: 0]\n"
    "  --maxsize           | largest file size (in bytes) to consider [default: infinity]\n"
    "  --pattern           | regular expression to test files against [default: .*]\n"
    "  --followdirsymlinks | enables following directory symlinks [default: off]\n"
    "  --out               | pathname of file to save output to\n"
  ;
}

int main(int const argc, char const *const *const argv) {
  if (argc == 1) {
    print_help_msg();
    EXIT(ExitCode::SUCCESS);
  }

  // process command line arguments
  bpo::variables_map options;
  try {
    bpo::store(
      bpo::parse_command_line(argc, argv, prgopt::options_descrip()),
      options
    );
  } catch (exception const &err) {
    cerr << "fatal: " << err.what() << '\n';
    EXIT(ExitCode::INVALID_ARGUMENT_SYNTAX);
  }
  bpo::notify(options);

  // attempt action and get result
  auto const result = [argv, &options]() {
    string const action = argv[1];
    if (action == "repeat") {
      return action::repeat(options);
    } else if (action == "sizerank") {
      return action::sizerank(options);
    } else {
      action::Result res(ExitCode::INVALID_ACTION);
      res.m_output  << "fatal: `" << action << "` is not a valid action";
      return res;
    }
  }();

  // print output
  {
    string const output = result.m_output.str();
    cout << result.m_output.str();
    if (!output.ends_with('\n')) {
      cout << '\n';
    }
  }

  EXIT(result.m_exitCode);
}
