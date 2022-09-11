#include "program-options.hpp"

namespace bpo = boost::program_options;

bpo::options_description const &prgopt::options_descrip() {
  static bpo::options_description desc("OPTIONS");
  static bool initialized = false;

  if (!initialized) {
    desc.add_options()
      ("in,i",    bpo::value<std::string>(), "")
      ("times,t", bpo::value<std::size_t>(), "")
      ("out,o",   bpo::value<std::string>(), "")
    ;
    initialized = true;
  }

  return desc;
}
