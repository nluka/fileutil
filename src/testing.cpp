#include <fstream>

// #include <boost/program_options.hpp>

#include "action.hpp"
#include "program-options.hpp"
#include "test.hpp"
#include "util.hpp"

// conflicts with `test` module
#undef assert

int main(void) {
  test::use_stdout(true);
  test::set_indentation("  ");
  test::set_verbose_mode(false);

  namespace bpo = boost::program_options;
  using namespace std;

  {
    SETUP_SUITE_USING(action::repeat)

    auto const testCase = [&s](char const *const name_, char const *const count) {
      string const
        name = string("repeat/") + name_,
        binInPathname = name + ".binin",
        binOutPathname = name + ".binout";

      char const *argv[] {
        "program_name_placeholder",
        "repeat",
        "--in", binInPathname.c_str(),
        "--count", count,
        "--out", binOutPathname.c_str()
      };

      bpo::variables_map options;
      bpo::store(
        bpo::parse_command_line(
          static_cast<int>(util::lengthof(argv)),
          argv,
          prgopt::options_descrip()
        ),
        options
      );

      action::Result const result = action::repeat(options);

      vector<char> binIn = util::extract_bin_file_contents(
        binInPathname.c_str()
      );
      vector<char> const binOut = util::extract_bin_file_contents(
        binOutPathname.c_str()
      );
      vector<char> const expectedBinOut = util::extract_bin_file_contents(
        (name + ".expectedbinout").c_str()
      );
      string const txtOut = result.m_output.str();
      string const expectedTxtOut = util::extract_txt_file_contents(
        (name + ".expectedtxtout").c_str()
      );

      s.assert(
        (string(name_) + " binouts match").c_str(),
        util::vectors_same(binOut, expectedBinOut)
      );
      s.assert(
        (string(name_) + " txtouts match").c_str(),
        txtOut.length() == expectedTxtOut.size() &&
        memcmp(txtOut.data(), expectedTxtOut.data(), txtOut.size()) == 0
      );
    };

    testCase("copy", "1");
    testCase("double", "2");
    testCase("triple", "3");
    testCase("quad", "4");

  } // repeat

  {
    SETUP_SUITE_USING(action::sizerank);

    auto const testCase = [&s](
      string const name,
      size_t const argc,
      char const *const *const argv,
      char const *const expected
    ) {
      bpo::variables_map options;
      bpo::store(
        bpo::parse_command_line(
          static_cast<int>(argc),
          argv,
          prgopt::options_descrip()
        ),
        options
      );

      action::Result const res = sizerank(options);
      string const out = res.m_output.str();

      s.assert(name.c_str(), out == expected);
    };

    {
      char const *argv[] {
        "program_name_placeholder",
        "sizerank",
        "--dir", "sizerank",
      };
      testCase(
        "defaults",
        util::lengthof(argv), argv,
        "1. (13.00 B) 13byte\n"
        "2. (12.00 B) _12byte\n"
        "3. (11.00 B) __11byte\n"
        "4. (10.00 B) _10byte\n"
        "5. (9.00 B) __9byte\n"
        "6. (8.00 B) _8byte\n"
        "7. (7.00 B) _7byte\n"
        "8. (6.00 B) _6byte\n"
        "9. (5.00 B) __5byte\n"
        "10. (4.00 B) __4byte\n"
      );
    }
    {
      char const *argv[] {
        "program_name_placeholder",
        "sizerank",
        "--dir", "sizerank",
        "--top", "3",
        "--minsize", "5",
        "--maxsize", "7",
        "--pattern", "_[0-9]+byte",
      };
      testCase(
        "top 3, [5, 7] bytes, start with single underscore",
        util::lengthof(argv), argv,
        "1. (7.00 B) _7byte\n"
        "2. (6.00 B) _6byte\n"
      );
    }

  } // sizerank

  test::evaluate_suites();

  return 0;
}
