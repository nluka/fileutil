#define _CRT_SECURE_NO_WARNINGS
#include <filesystem>
#include <stdexcept>
#include <sstream>

#include "util.hpp"

std::vector<std::uint8_t> util::extract_file(
  std::string const &pathname,
  bool const binaryMode
) {
  bool const fileExists = std::filesystem::exists(pathname);
  if (!fileExists) {
    std::stringstream err{};
    err << "file `" << pathname << "` not found";
    throw std::runtime_error(err.str());
  }

  FILE *file = fopen(pathname.c_str(), binaryMode ? "rb" : "r");
  if (file == nullptr) {
    std::stringstream err{};
    err << "failed to open file `" << pathname << "`";
    throw std::runtime_error(err.str());
  }

  auto const fileSize = static_cast<std::size_t>(
    std::filesystem::file_size(pathname)
  );
  std::vector<std::uint8_t> buffer(fileSize);
  fread(buffer.data(), 1, fileSize, file);

  return buffer;
}
