#include <array>
#include <iostream>
#include <string>
#include <utility>
#include <functional>

#include "action.hpp"

int main(int const argc, char const *const *const argv) {
  struct action_bundle {
    std::string name;
    std::function< std::string (void) > help_fn;
    std::function< std::string (int, char const *const *) > perform_fn;
  };

  action_bundle const actions[] {
    { "repeat", action::repeat_help_msg, action::repeat_perform },
    { "sizerank", action::sizerank_help_msg, action::sizerank_perform },
  };

  if (argc < 2) {
    std::string actions_str{};
    for (auto const &action : actions) {
      actions_str += action.name;
      actions_str += '|';
    }
    actions_str.pop_back(); // remove trailing |

    std::cout << "\n usage: \n"
      << "  fileutil " << actions_str << " help\n"
      << "  fileutil " << actions_str << " [options...]\n\n";

    return 0;
  }

  std::string const first_arg = argv[1];
  std::string const second_arg = argc >= 3 ? argv[2] : "";

  for (auto const &[name, help_fn, perform_fn] : actions) {
    if (first_arg == name) {
      std::string out{};
      if (second_arg == "help")
        out = help_fn();
      else
        out = perform_fn(argc, argv);

      std::cout << '\n' << out << '\n';

      return 0;
    }
  }

  return -1;
}
