#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "action.hpp"

using namespace std;

action::Result action::repeat(boost::program_options::variables_map const &options) {
  #define CHECK_OPT_EXISTS(opt) \
  if (!options.count(opt)) { \
    action::Result res(ExitCode::MISSING_REQUIRED_OPTION); \
    res.m_output << "fatal: missing required option `" opt "`"; \
    return res; \
  }

  CHECK_OPT_EXISTS("in")
  auto const in = options.at("in").as<string>();

  CHECK_OPT_EXISTS("count")
  auto const count = options.at("count").as<size_t>();
  if (count == 0) {
    action::Result res(ExitCode::BAD_OPTION_VALUE);
    res.m_output << "fatal: option `count` must be > 0";
    return res;
  }

  CHECK_OPT_EXISTS("out")
  auto const out = options.at("out").as<string>();

  #undef CHECK_OP_EXISTS

  ifstream inFile(in, ios::binary);
  if (!inFile.is_open()) {
    Result res(ExitCode::FILE_OPEN_FAILED);
    res.m_output << "fatal: failed to open file `" << in << "`";
    return res;
  }

  ofstream outFile(out, ios::binary);
  if (!inFile.is_open()) {
    Result res(ExitCode::FILE_OPEN_FAILED);
    res.m_output << "fatal: failed to open file `" << out << "`";
    return res;
  }

  auto const inFileSizeInBytes = static_cast<size_t>(
    filesystem::file_size(in)
  );
  size_t bufferSize = min(
    static_cast<size_t>(2 * 1024 * 1024),
    inFileSizeInBytes
  );
  vector<uint8_t> buffer(bufferSize);

  for (size_t i = 1; i <= count; ++i) {
    size_t numBytesReadSoFar = 0;
    inFile.seekg(ios::beg, 0);

    while (numBytesReadSoFar < inFileSizeInBytes) {
      size_t const numBytesRemaining = inFileSizeInBytes - numBytesReadSoFar;
      size_t const numBytesToProcessThisIter =
        numBytesRemaining >= bufferSize
          ? bufferSize
          : numBytesRemaining
      ;
      inFile.read(
        reinterpret_cast<char *>(buffer.data()),
        numBytesToProcessThisIter
      );
      outFile.write(
        reinterpret_cast<char const *>(buffer.data()),
        numBytesToProcessThisIter
      );
      numBytesReadSoFar += numBytesToProcessThisIter;
    }
  }

  return Result(ExitCode::SUCCESS, "success");
}
