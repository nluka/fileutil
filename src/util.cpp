#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstdarg>

#include <boost/program_options.hpp>

#include "util.hpp"

using namespace std;

string util::make_str(char const *const fmt, ...) {
  size_t const buf_size = 1024;
  char buffer[buf_size];

  va_list args;
  va_start(args, fmt);
  [[maybe_unused]] int const cnt = vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  return std::string(buffer);
}

fstream util::open_file(char const *const file_path, int const flags) {
  bool const for_reading = (flags & 1) == 1;

  if (for_reading && !filesystem::exists(file_path))
    throw std::runtime_error(make_str("file '%s' not found", file_path));

  fstream file(file_path, static_cast<ios_base::openmode>(flags));

  if (!file.is_open())
    throw std::runtime_error(make_str("unable to open file '%s'", file_path));

  if (!file)
    throw std::runtime_error(make_str("file '%s' in bad state", file_path));

  return file;
}

vector<char> util::extract_bin_file_contents(char const *const file_path) {
  fstream file = util::open_file(file_path, ios::binary | ios::in);
  auto const file_size = filesystem::file_size(file_path);
  vector<char> vec(file_size);
  file.read(vec.data(), file_size);
  return vec;
}

string util::extract_txt_file_contents(char const *const file_path) {
  fstream file = util::open_file(file_path, ios::in);
  auto const file_size = filesystem::file_size(file_path);

  string content{};
  content.reserve(file_size);

  getline(file, content, '\0');

  // remove any \r characters
  content.erase(
    remove(content.begin(), content.end(), '\r'),
    content.end()
  );

  return content;
}

string util::format_file_size(uintmax_t const size) {
  char out[21];
  format_file_size(size, out, sizeof(out));
  return string(out);
}

void util::format_file_size(
  uintmax_t const file_size,
  char *const out,
  size_t const out_size
) {
  char const *units[] = { "B", "KB", "MB", "GB", "TB" };
  size_t constexpr largest_unit_idx = util::lengthof(units) - 1;
  size_t unit_idx = 0;

  double size = static_cast<double>(file_size);

  while (size >= 1024 && unit_idx < largest_unit_idx) {
    size /= 1024;
    ++unit_idx;
  }

  char const *const fmt =
    unit_idx == 0
    // no digits after decimal point for bytes
    // because showing a fraction of a byte doesn't make sense
    ? "%.0lf %s"
    // 2 digits after decimal points for denominations
    // greater than bytes
    : "%.2lf %s";

  snprintf(out, out_size, fmt, size, units[unit_idx]);
}
