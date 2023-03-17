#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "util.hpp"
#include "action.hpp"

namespace fs = std::filesystem;
namespace bpo = boost::program_options;

bpo::options_description action::repeat_options_desc() {
  bpo::options_description desc("REPEAT OPTIONS");
  desc.add_options()
    ("inpath,i", bpo::value<std::string>(), "Path of file to repeat")
    ("outpath,o", bpo::value<std::string>(), "Path of resultant file")
    ("repeats,n", bpo::value<size_t>(), "Number of times to duplicate content, 1 = copy")
    ;
  return desc;
}

std::string action::repeat_help_msg() {
  std::stringstream out{};

  out << '\n';
  repeat_options_desc().print(out, 6);
  out << '\n';

  return out.str();
}

struct repeat_config {
  fs::path in_path;
  fs::path out_path;
  size_t num_repeats;
};

static
repeat_config parse_config(bpo::variables_map const& var_map, std::vector<std::string>& errors) {
  using util::get_required_option;
  using util::get_nonrequired_option;
  using util::get_flag_option;

  repeat_config cfg{};
  {
    auto in_path = get_required_option<std::string>("inpath", "i", var_map, errors);

    if (in_path.has_value()) {
      fs::path p = in_path.value();
      if (!fs::exists(p)) {
        errors.emplace_back("(--inpath, -i) file not found");
      } else if (!fs::is_regular_file(p)) {
        errors.emplace_back("(--inpath, -i) is not a regular file");
      } else {
        cfg.in_path = std::move(p);
      }
    }
  }
  {
    auto out_path = get_required_option<std::string>("outpath", "o", var_map, errors);

    if (out_path.has_value()) {
      std::ofstream f(out_path.value());
      if (!f.is_open()) {
        errors.emplace_back("(--outpath, -o) file cannot be opened");
      }
      else {
        cfg.out_path = std::move(out_path.value());
      }
    }
  }
  {
    auto num_repeats = get_required_option<size_t>("repeats", "n", var_map, errors);
    if (num_repeats.has_value()) {
      if (num_repeats.value() == 0) {
        errors.emplace_back("(--repeats, -n) value must be > 0");
      } else {
        cfg.num_repeats = num_repeats.value();
      }
    }
  }

  return cfg;
}

std::string action::repeat_perform(int const argc, char const* const* const argv) {
  std::stringstream out{};

  bpo::variables_map var_map;
  try {
    bpo::store(bpo::parse_command_line(argc, argv, action::repeat_options_desc()), var_map);
  }
  catch (std::exception const& err) {
    out << err.what() << '\n';
    return out.str();
  }
  bpo::notify(var_map);

  std::vector<std::string> errors{};
  repeat_config const cfg = parse_config(var_map, errors);
  if (!errors.empty()) {
    for (auto const& err : errors)
      out << err << '\n';
    return out.str();
  }

  std::ifstream in_file(cfg.in_path, std::ios::binary);
  if (!in_file.is_open()) {
    out << "fatal: failed to open file " << cfg.in_path << '\n';
    return out.str();
  }

  std::ofstream out_file(cfg.out_path, std::ios::binary);
  if (!out_file.is_open()) {
    out << "fatal: failed to open file " << cfg.out_path << '\n';
    return out.str();
  }

  auto const in_file_size = static_cast<size_t>(fs::file_size(cfg.in_path));
  size_t const buf_size = std::min(static_cast<size_t>(2 * 1024 * 1024), in_file_size);
  std::vector<std::byte> buffer(buf_size);

  for (size_t i = 1; i <= cfg.num_repeats; ++i) {
    size_t num_bytes_read_thus_far = 0;
    in_file.seekg(std::ios::beg, 0);

    while (num_bytes_read_thus_far < in_file_size)  {
      size_t const num_bytes_remaining = in_file_size - num_bytes_read_thus_far;

      size_t const num_bytes_to_process_this_iteration =
        num_bytes_remaining >= buf_size
        ? buf_size
        : num_bytes_remaining;

      in_file.read(
        reinterpret_cast<char *>(buffer.data()),
        num_bytes_to_process_this_iteration);

      out_file.write(
        reinterpret_cast<char const*>(buffer.data()),
        num_bytes_to_process_this_iteration);

      num_bytes_read_thus_far += num_bytes_to_process_this_iteration;
    }
  }

  out << "Successfully repeated " << cfg.in_path << ' '
    << cfg.num_repeats << " times as " << cfg.out_path << '\n';
  return out.str();
}
