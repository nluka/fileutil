#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "action.hpp"

action::Result action::repeat(boost::program_options::variables_map const &options) {
  #define CHECK_OPT_EXISTS(opt) \
  if (!options.count(opt)) { \
    Result res(ExitCode::MISSING_REQUIRED_OPTION); \
    res.m_output << "fatal: missing required option `" opt "`"; \
    return res; \
  }

  CHECK_OPT_EXISTS("in")
  auto const in = options.at("in").as<std::string>();

  CHECK_OPT_EXISTS("times")
  auto const times = options.at("times").as<std::size_t>();

  CHECK_OPT_EXISTS("out")
  auto const out = options.at("out").as<std::string>();

  std::ifstream inFile(in, std::ios::binary);
  if (!inFile.is_open()) {
    Result res(ExitCode::FILE_OPEN_FAILED);
    res.m_output << "fatal: failed to open file `" << in << "`";
    return res;
  }

  std::ofstream outFile(out, std::ios::binary);
  if (!inFile.is_open()) {
    Result res(ExitCode::FILE_OPEN_FAILED);
    res.m_output << "fatal: failed to open file `" << out << "`";
    return res;
  }

  auto const inFileSizeInBytes = static_cast<std::size_t>(
    std::filesystem::file_size(in)
  );
  std::size_t bufferSize = std::min(
    static_cast<std::size_t>(2 * 1024 * 1024),
    inFileSizeInBytes
  );
  std::vector<std::uint8_t> buffer(bufferSize);

  for (std::size_t i = 0; i <= times; ++i) {
    std::size_t numBytesReadSoFar = 0;
    inFile.seekg(std::ios::beg, 0);

    while (numBytesReadSoFar < inFileSizeInBytes) {
      std::size_t const numBytesRemaining = inFileSizeInBytes - numBytesReadSoFar;
      std::size_t const numBytesToProcessThisIter =
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
