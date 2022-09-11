#include <fstream>

#include <boost/program_options.hpp>

#include "action.hpp"
#include "program-options.hpp"
#include "test.hpp"
#include "util.hpp"

#undef assert

int main() {
  test::use_stdout(true);
  test::set_indentation("  ");
  test::set_verbose_mode(false);
  // SETUP_SUITE_USING(action::repeat)

  auto const testCase = [](
    std::string const name,
    char const *const repeats
  ) {
    namespace bpo = boost::program_options;

    test::Suite s(name);

    std::string const binInPathname = name + ".binin";
    std::string const binOutPathname = name + ".binout";

    char const *argv[] {
      "program_name",
      "-i", binInPathname.c_str(),
      "-t", repeats,
      "-o", binOutPathname.c_str()
    };

    bpo::variables_map options;
    bpo::store(
      bpo::parse_command_line(7, argv, prgopt::options_descrip()),
      options
    );

    auto const result = action::repeat(options);
    std::string const txtOut = result.m_output.str();

    auto const binIn = util::extract_file(binInPathname, true);
    auto const expectedBinOut = util::extract_file(name + ".expectedbinout", true);
    auto const expectedTxtOut = util::extract_file(name + ".expectedtxtout");
    auto const binOut = util::extract_file(binOutPathname, true);

    s.assert(
      "binouts match",
      util::vectors_same(binOut, expectedBinOut)
    );
    s.assert(
      "txtouts match",
      txtOut.length() == expectedTxtOut.size() &&
      std::memcmp(txtOut.data(), expectedTxtOut.data(), txtOut.size()) == 0
    );

    test::register_suite(std::move(s));
  };

  testCase("dup", "0");
  testCase("once", "1");
  testCase("twice", "2");
  testCase("thrice", "3");

  test::evaluate_suites();

  return 0;
}
