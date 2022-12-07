#include <fstream>

#include "action.hpp"
#include "program-options.hpp"
#include "test.hpp"
#include "util.hpp"

#undef assert // due to conflict with `test` module

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
        "1. (13 B) 13byte\n"
        "2. (12 B) _12byte\n"
        "3. (11 B) __11byte\n"
        "4. (10 B) _10byte\n"
        "5. (9 B) __9byte\n"
        "6. (8 B) _8byte\n"
        "7. (7 B) _7byte\n"
        "8. (6 B) _6byte\n"
        "9. (5 B) __5byte\n"
        "10. (4 B) __4byte\n"
      );
    }
    {
      char const *argv[] {
        "program_name_placeholder",
        "sizerank",
        "--dir", "sizerank",
        "--recursive",
        "--top", "4",
        "--minsize", "3",
        "--maxsize", "7",
        "--pattern", "_[0-9]+byte",
      };
      testCase(
        "top 5, [3, 7] bytes, starts with single underscore",
        util::lengthof(argv), argv,
        "1. (7 B) _7byte\n"
        "2. (6 B) _6byte\n"
        "3. (4 B) subdir\\_4byte\n"
        "4. (3 B) _3byte\n"
      );
    }

  } // sizerank

  {
    SETUP_SUITE_USING(util::format_file_size)

    auto const testCase = [&s](
      uintmax_t const size,
      char const *const expected
    ) {
      string const result = format_file_size(size);
      stringstream name{};
      name << size << " == " << expected << " (got " << result << ")";
      s.assert(name.str().c_str(), result == expected);
    };

    testCase(0, "0 B");

    testCase(1023, "1023 B");
    testCase(1024, "1.00 KB");
    testCase(2048, "2.00 KB");

    testCase(1'048'575, "1024.00 KB");
    testCase(1'048'576, "1.00 MB");

    testCase(1'073'741'823, "1024.00 MB");
    testCase(1'073'741'824, "1.00 GB");

    testCase(1'099'511'627'775, "1024.00 GB");
    testCase(1'099'511'627'776, "1.00 TB");
  }

  test::evaluate_suites();

  return 0;
}
