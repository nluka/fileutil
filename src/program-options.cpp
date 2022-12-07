#include "program-options.hpp"

namespace bpo = boost::program_options;

bpo::options_description const &prgopt::options_descrip() {
  static bpo::options_description desc("OPTIONS");
  static bool initialized = false;
  char const *const emptyDescrip = "";

  if (!initialized) {
    using namespace std;
    using bpo::value;
    desc.add_options()
      // repeat
      ("in", value<string>(), emptyDescrip)
      ("count", value<size_t>(), emptyDescrip)
      ("out", value<string>(), emptyDescrip)
      // sizerank
      ("dir", value<string>(), emptyDescrip)
      ("recursive", emptyDescrip)
      ("top", value<uintmax_t>(), emptyDescrip)
      ("minsize", value<uintmax_t>(), emptyDescrip)
      ("maxsize", value<uintmax_t>(), emptyDescrip)
      ("pattern", value<string>(), emptyDescrip)
      ("followdirsymlinks", emptyDescrip)
    ;
    initialized = true;
  }

  return desc;
}
