#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace util {

std::vector<std::uint8_t> extract_file(
  std::string const &pathname,
  bool binaryMode = false
);

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

} // namespace util

#endif // UTIL_HPP
