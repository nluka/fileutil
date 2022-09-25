#define _CRT_SECURE_NO_WARNINGS
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <sstream>

#include "exit.hpp"
#include "util.hpp"

using namespace std;

uint16_t util::byteswap_uint16(uint16_t const val) {
  #if MICROSOFT_COMPILER
    static_assert(sizeof(unsigned short) == sizeof(uint16_t));
    return _byteswap_ushort(val);
  #elif GXX_COMPILER
    return __builtin_bswap16(val);
  #else
    #error "unsupported compiler"
  #endif
}

uint32_t util::byteswap_uint32(uint32_t const val) {
  #if MICROSOFT_COMPILER
    static_assert(sizeof(unsigned long) == sizeof(uint32_t));
    return _byteswap_ulong(val);
  #elif GXX_COMPILER
    return __builtin_bswap32(val);
  #else
    #error "unsupported compiler"
  #endif
}

fstream util::open_file(char const *const pathname, int const flags) {
  bool const forReading = (flags & 1) == 1;
  if (forReading) {
    if (!filesystem::exists(pathname)) {
      cerr << "fatal: file `" << pathname << "` not found\n";
      EXIT(ExitCode::FILE_NOT_FOUND);
    }
  }

  fstream file(pathname, static_cast<ios_base::openmode>(flags));

  if (!file.is_open()) {
    cerr << "fatal: unable to open file `" << pathname << "`\n";
    EXIT(ExitCode::FILE_OPEN_FAILED);
  }

  if (!file.good()) {
    cerr << "fatal: bad file `" << pathname << "`\n";
    EXIT(ExitCode::BAD_FILE);
  }

  return file;
}

vector<char> util::extract_bin_file_contents(char const *const pathname) {
  fstream file = util::open_file(pathname, ios::binary | ios::in);
  auto const fileSize = filesystem::file_size(pathname);
  vector<char> vec(fileSize);
  file.read(vec.data(), fileSize);
  return vec;
}

string util::extract_txt_file_contents(char const *const pathname) {
  fstream file = util::open_file(pathname, ios::in);
  auto const fileSize = filesystem::file_size(pathname);

  string content{};
  content.reserve(fileSize);

  getline(file, content, '\0');

  // remove any \r characters
  content.erase(
    remove(content.begin(), content.end(), '\r'),
    content.end()
  );

  return content;
}

void util::format_file_size(
  uintmax_t const size_,
  char *const out,
  size_t const outSize
) {
  char const *units[] = { "B", "KB", "MB", "GB", "TB" };
  size_t constexpr largestUnitIdx = util::lengthof(units) - 1;
  size_t unitIdx = 0;

  double size = static_cast<double>(size_);

  while (size >= 1024 && unitIdx < largestUnitIdx) {
    size /= 1024;
  }

  snprintf(out, outSize, "%.2f %s", size, units[unitIdx]);
}
