#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#ifdef _MSC_VER
  #define MICROSOFT_COMPILER 1
#elif __GNUC__
  #define GXX_COMPILER 1
#endif

#ifdef MICROSOFT_COMPILER
#include <cstdlib>
#endif

namespace util {

std::uint16_t byteswap_uint16(std::uint16_t val);
std::uint32_t byteswap_uint32(std::uint32_t val);

std::fstream open_file(char const *pathname, int flags);
std::vector<char> extract_bin_file_contents(char const *pathname);
std::string extract_txt_file_contents(char const *pathname);

template <typename ElemTy>
bool vectors_same(
  std::vector<ElemTy> const &v1,
  std::vector<ElemTy> const &v2
) {
  if (v1.size() != v2.size()) {
    return false;
  } else {
    return std::memcmp(v1.data(), v2.data(), v1.size()) == 0;
  }
}

// Returns the size of a static (stack-allocated) C-style array at compile time.
template <typename ElemTy, std::size_t Length>
constexpr
std::size_t lengthof(ElemTy (&)[Length]) {
  // implementation from: https://stackoverflow.com/a/2404697/16471560
  return Length;
}

std::string format_file_size(std::uintmax_t size);
void format_file_size(std::uintmax_t size, char *out, std::size_t outSize);

} // namespace util

#endif // UTIL_HPP
