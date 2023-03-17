#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <optional>

#ifdef _MSC_VER
  #define MICROSOFT_COMPILER 1
#elif __GNUC__
  #define GXX_COMPILER 1
#endif

#ifdef MICROSOFT_COMPILER
#include <cstdlib>
#endif

namespace util {

std::fstream open_file(char const *pathname, int flags);
std::vector<char> extract_bin_file_contents(char const *pathname);
std::string extract_txt_file_contents(char const *pathname);

std::string make_str(char const *const fmt, ...);

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
consteval [[nodiscard]] std::size_t lengthof(ElemTy (&)[Length]) {
  return Length;
}

[[nodiscard]] std::string format_file_size(std::uintmax_t size);
void format_file_size(std::uintmax_t size, char *out, std::size_t outSize);

template <typename Ty>
[[nodiscard]] std::optional<Ty> get_required_option(
  char const *const full_name,
  char const *const short_name,
  boost::program_options::variables_map const &var_map,
  std::vector<std::string> &errors)
{
  if (var_map.count(full_name) == 0) {
      errors.emplace_back(make_str("(--%s, -%s) required option missing", full_name, short_name));
      return std::nullopt;
  }

  try {
      return var_map.at(full_name).as<Ty>();
  } catch (...) {
      errors.emplace_back(make_str("(--%s, -%s) unable to parse value", full_name, short_name));
      return std::nullopt;
  }
}

template <typename Ty>
[[nodiscard]] std::optional<Ty> get_nonrequired_option(
  char const *const full_name,
  char const *const short_name,
  boost::program_options::variables_map const &var_map,
  std::vector<std::string> &errors)
{
  if (var_map.count(full_name) == 0) {
      return std::nullopt;
  }

  try {
      return var_map.at(full_name).as<Ty>();
  } catch (...) {
      errors.emplace_back(make_str("(--%s , -%s) unable to parse value", full_name, short_name));
      return std::nullopt;
  }
}

inline
[[nodiscard]] bool get_flag_option(
  char const *const option_name,
  boost::program_options::variables_map const &var_map)
{
  return var_map.count(option_name) > 0;
}

} // namespace util

#endif // UTIL_HPP
