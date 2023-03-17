#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include "action.hpp"
#include "ntest.hpp"
#include "util.hpp"

namespace bpo = boost::program_options;
using namespace std;

int main(void) {
  ntest::init();
  ntest::config::set_max_arr_preview_len(2);
  ntest::config::set_max_str_preview_len(10);

  // repeat
  {
    {
      char const *argv[] {
        "program_name_placeholder",
        "repeat",
      };
      std::string const out = action::repeat_perform((int)util::lengthof(argv), argv);
      ntest::assert_cstr(
        (
          "(--inpath, -i) required option missing\n"
          "(--outpath, -o) required option missing\n"
          "(--repeats, -n) required option missing\n"
        ),
        out.c_str()
      );
    }
    {
      char const *argv[] {
        "program_name_placeholder",
        "repeat",
        "-i", "does_not_exist",
        "-n", "0",
        "-o", "bad_dir/out.bin",
      };
      std::string const out = action::repeat_perform((int)util::lengthof(argv), argv);
      ntest::assert_cstr(
        (
          "(--inpath, -i) file not found\n"
          "(--outpath, -o) file cannot be opened\n"
          "(--repeats, -n) value must be > 0\n"
        ),
        out.c_str()
      );
    }
    {
      char const *argv[] {
        "program_name_placeholder",
        "repeat",
        "-i", "repeat/copy.binin",
        "-n", "1",
        "-o", "repeat/copy.binout",
      };
      std::string const out = action::repeat_perform((int)util::lengthof(argv), argv);
      ntest::assert_cstr("Successfully repeated \"repeat/copy.binin\" 1 times as \"repeat/copy.binout\"\n", out.c_str());
      ntest::assert_binary_file("repeat/copy.expectedbinout", "repeat/copy.binout");
    }
    {
      char const *argv[] {
        "program_name_placeholder",
        "repeat",
        "-i", "repeat/double.binin",
        "-n", "2",
        "-o", "repeat/double.binout",
      };
      std::string const out = action::repeat_perform((int)util::lengthof(argv), argv);
      ntest::assert_cstr("Successfully repeated \"repeat/double.binin\" 2 times as \"repeat/double.binout\"\n", out.c_str());
      ntest::assert_binary_file("repeat/double.expectedbinout", "repeat/double.binout");
    }
    {
      char const *argv[] {
        "program_name_placeholder",
        "repeat",
        "-i", "repeat/triple.binin",
        "-n", "3",
        "-o", "repeat/triple.binout",
      };
      std::string const out = action::repeat_perform((int)util::lengthof(argv), argv);
      ntest::assert_cstr("Successfully repeated \"repeat/triple.binin\" 3 times as \"repeat/triple.binout\"\n", out.c_str());
      ntest::assert_binary_file("repeat/triple.expectedbinout", "repeat/triple.binout");
    }
    {
      char const *argv[] {
        "program_name_placeholder",
        "repeat",
        "-i", "repeat/quad.binin",
        "-n", "4",
        "-o", "repeat/quad.binout",
      };
      std::string const out = action::repeat_perform((int)util::lengthof(argv), argv);
      ntest::assert_cstr("Successfully repeated \"repeat/quad.binin\" 4 times as \"repeat/quad.binout\"\n", out.c_str());
      ntest::assert_binary_file("repeat/quad.expectedbinout", "repeat/quad.binout");
    }
  }

  // sizerank
  {
    {
      char const *argv[] {
        "program_name_placeholder",
        "sizerank",
        "--dir", "does_not_exist",
        "--sizelim", "3,2",
        "--pattern", "*",
        "--outpath", "bad_dir/out.txt",
      };
      std::string const out = action::sizerank_perform((int)util::lengthof(argv), argv);
      ntest::assert_cstr(
        (
          "(--pattern, -p) is not a invalid regular expression\n"
          "(--dir, -d) is not a directory\n"
          "(--outpath, -o) cannot be opened\n"
          "(--sizelim, -s) max(rhs) must be >= min(lhs)\n"
        ),
        out.c_str()
      );
    }
    {
      char const *argv[] {
        "program_name_placeholder",
        "sizerank",
        "--dir", "sizerank",
      };
      std::string const out = action::sizerank_perform((int)util::lengthof(argv), argv);
      ntest::assert_cstr(
        (
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
        ),
        out.c_str()
      );
    }
    {
      char const *argv[] {
        "program_name_placeholder",
        "sizerank",
        "--dir", "sizerank",
        "--recurse",
        "--top", "4",
        "--sizelim", "3,7",
        "--pattern", "_[0-9]+byte",
      };
      std::string const out = action::sizerank_perform((int)util::lengthof(argv), argv);
      ntest::assert_cstr(
        (
          "1. (7 B) _7byte\n"
          "2. (6 B) _6byte\n"
          "3. (4 B) subdir\\_4byte\n"
          "4. (3 B) _3byte\n"
        ),
        out.c_str()
      );
    }
  } // sizerank

  {
    using util::format_file_size;

    ntest::assert_stdstr("0 B", format_file_size(0));
    ntest::assert_stdstr("1023 B", format_file_size(1023));
    ntest::assert_stdstr("1.00 KB", format_file_size(1024));
    ntest::assert_stdstr("2.00 KB", format_file_size(2048));
    ntest::assert_stdstr("1024.00 KB", format_file_size(1'048'575));
    ntest::assert_stdstr("1.00 MB", format_file_size(1'048'576));
    ntest::assert_stdstr("1024.00 MB", format_file_size(1'073'741'823));
    ntest::assert_stdstr("1.00 GB", format_file_size(1'073'741'824));
    ntest::assert_stdstr("1024.00 GB", format_file_size(1'099'511'627'775));
    ntest::assert_stdstr("1.00 TB", format_file_size(1'099'511'627'776));
  }

  auto const res = ntest::generate_report("fileutil");
  std::cout << res.num_passes << " passed, " << res.num_fails << " failed";
  return static_cast<int>(res.num_fails);
}
