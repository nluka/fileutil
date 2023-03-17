#include <filesystem>
#include <functional>
#include <regex>
#include <vector>
#include <iostream>
#include <sstream>

#include <boost/program_options.hpp>

#include "action.hpp"
#include "util.hpp"

namespace bpo = boost::program_options;
namespace fs = std::filesystem;

bpo::options_description action::sizerank_options_desc() {
  bpo::options_description desc("SIZERANK OPTIONS");
  desc.add_options()
    ("dir,d", bpo::value<std::string>(), "Search directory, default=cwd")
    ("recurse,r", "Enables recursive search through child directories, default=false")
    ("top,n", bpo::value<size_t>(), "Number of top entries to rank, default=10")
    ("sizelim,s", bpo::value<std::string>(), "File size limits to consider, inclusive, format: min,max")
    ("pattern,p", bpo::value<std::string>(), "Regular expression to match file names against, default=.*")
    ("followsymlinks,l", "Enables following symbolic links, default=false")
    ("outpath,o", bpo::value<std::string>(), "Path of output file, default=none")
  ;
  return desc;
}

std::string action::sizerank_help_msg() {
  std::stringstream out{};

  out << '\n';
  sizerank_options_desc().print(out, 6);
  out << '\n';

  return out.str();
}

struct sizerank_config {
  std::string pattern;
  std::string search_path;
  std::string out_path;
  size_t top_n;
  size_t min_size;
  size_t max_size;
  bool recurse;
  bool follow_sym_links;
};

static
sizerank_config parse_config(bpo::variables_map const &var_map, std::vector<std::string> &errors) {
  using util::get_required_option;
  using util::get_nonrequired_option;
  using util::get_flag_option;

  sizerank_config cfg{};

  {
    auto pattern = get_nonrequired_option<std::string>("pattern", "p", var_map, errors);

    if (pattern.has_value()) {
      try {
        // if pattern is not a valid regexp, std::regex ctor will throw
        [[maybe_unused]] std::regex const r(pattern.value());

        // construction succeeded, thus pattern is a valid regexp
        cfg.pattern = std::move(pattern.value());
      } catch (...) {
        errors.emplace_back("(--pattern, -p) is not a invalid regular expression");
      }
    } else {
      cfg.pattern = ".*";
    }
  }
  {
    auto search_path = get_nonrequired_option<std::string>("dir", "d", var_map, errors);

    if (search_path.has_value()) {
      if (!fs::is_directory(search_path.value())) {
        errors.emplace_back("(--dir, -d) is not a directory");
      } else {
        cfg.search_path = std::move(search_path.value());
      }
    } else {
      cfg.search_path = fs::current_path().string();
    }
  }
  {
    auto out_path = get_nonrequired_option<std::string>("outpath", "o", var_map, errors);

    if (out_path.has_value()) {
      std::ofstream f(out_path.value());
      if (!f.is_open()) {
        errors.emplace_back("(--outpath, -o) cannot be opened");
      } else {
        cfg.out_path = std::move(out_path.value());
      }
    } else {
      cfg.out_path = "";
    }
  }
  {
    auto top_n = get_nonrequired_option<size_t>("top", "n", var_map, errors);
    cfg.top_n = top_n.value_or(10);
  }
  {
    auto size_lim = get_nonrequired_option<std::string>("sizelim", "s", var_map, errors);

    if (size_lim.has_value()) {
      char const *const valid_regex = "^[0-9]+,[0-9]+$";
      if (!std::regex_match(size_lim.value(), std::regex(valid_regex))) {
        errors.emplace_back(util::make_str("(--sizelim, -s) must match /%s/", valid_regex));
      } else {
        // since we verified the format of `size_lim`,
        // there shouldn't be any exceptions caused by this block...

        size_t const comma_pos = size_lim.value().find_first_of(',');
        assert(comma_pos != std::string::npos);
        size_lim.value()[comma_pos] = '\0';

        char const *const min_cstr = size_lim.value().c_str();
        char const *const max_cstr = min_cstr + comma_pos + 1;

        size_t const min = std::stoull(min_cstr);
        size_t const max = std::stoull(max_cstr);

        if (max == 0) {
          errors.emplace_back("(--sizelim, -s) max must be > 0");
        } else if (max < min) {
          errors.emplace_back("(--sizelim, -s) max(rhs) must be >= min(lhs)");
        } else {
          cfg.min_size = min;
          cfg.max_size = max;
        }
      }
    } else {
      cfg.min_size = 0;
      cfg.max_size = UINT64_MAX;
    }
  }
  {
    bool const recurse = get_flag_option("recurse", var_map);
    cfg.recurse = recurse;
  }
  {
    bool const follow_sym_links = get_flag_option("followsymlinks", var_map);
    cfg.follow_sym_links = follow_sym_links;
  }

  return cfg;
}

bool smallest(uintmax_t const curr_file_sz, uintmax_t const lowest_ranked_file_sz) {
  return curr_file_sz < lowest_ranked_file_sz;
}
bool largest(uintmax_t const curr_file_sz, uintmax_t const lowest_ranked_file_sz) {
  return curr_file_sz > lowest_ranked_file_sz;
}

std::string action::sizerank_perform(int const argc, char const *const *const argv) {
  std::stringstream out_ss{};

  bpo::variables_map var_map;
  try {
    bpo::store(bpo::parse_command_line(argc, argv, action::sizerank_options_desc()), var_map);
  } catch (std::exception const &err) {
    out_ss << err.what() << '\n';
    return out_ss.str();
  }
  bpo::notify(var_map);

  std::vector<std::string> errors{};
  sizerank_config cfg = parse_config(var_map, errors);
  if (!errors.empty()) {
    for (auto const &err : errors)
      out_ss << err << '\n';
    return out_ss.str();
  }

  struct file_entry {
    fs::path m_path;
    uintmax_t m_size;
  };

  size_t num_files_found = 0;
  std::vector<file_entry> top_files{};
  try {
    top_files.reserve(cfg.top_n + 1); // 1 extra for when we overflow
  } catch (std::exception const &except) {
    out_ss << except.what() << '\n';
    return out_ss.str();
  }

  auto const binary_insert = [&top_files](
    fs::path const &path,
    uintmax_t const size
  ) {
    if (top_files.empty()) {
      top_files.emplace_back(path, size);
      return;
    }

    int64_t first = 0;
    int64_t last = static_cast<uint64_t>(top_files.size() - 1);

    while (last - first > 1) {
      int64_t const middle = (first + last) / 2;
      if (size < top_files[middle].m_size)
        first = middle;
      else
        last = middle;
    }

    std::vector<file_entry>::const_iterator const insert_pos = [&]() {
      if (size > top_files[first].m_size)
        return top_files.cbegin() + first;
      else if (size < top_files[last].m_size)
        return top_files.cbegin() + last + 1;
      else
        return top_files.cbegin() + last;
    }();

    top_files.emplace(insert_pos, path, size);
  };

  std::regex const pattern_regex(cfg.pattern);

  auto const process_dir_entry = [&](fs::directory_entry const &entry) {
    if (entry.is_directory()) {
      return;
    }

    std::error_code ec{};
    uintmax_t const size = fs::file_size(entry, ec);
    if (ec) {
      return;
    }

    ++num_files_found;

    // quickest check, do it first
    if (size < cfg.min_size || size > cfg.max_size) {
      return;
    }

    // second quickest check, do it second
    {
      bool const top_files_is_full = top_files.size() == cfg.top_n;
      if (top_files_is_full) {
        uintmax_t const lowest_ranked_sz = top_files[top_files.size() - 1].m_size;
        if (size < lowest_ranked_sz) {
          return;
        }
      }
    }

    // slowest check, do it last
    if (!cfg.pattern.empty()) {
      bool const fname_matches_pattern = std::regex_match(
        entry.path().filename().string(),
        pattern_regex);

      if (!fname_matches_pattern) {
        return;
      }
    }

    binary_insert(entry, size);
    if (top_files.size() > cfg.top_n) {
      top_files.pop_back();
    }
  };

  fs::directory_options dir_options = fs::directory_options::skip_permission_denied;
  if (cfg.follow_sym_links) {
    dir_options |= fs::directory_options::follow_directory_symlink;
  }

  // find top files
  if (cfg.recurse) {
    // process each directory entry, and any child directories
    for (
      auto const &entry :
      fs::recursive_directory_iterator(cfg.search_path, dir_options)
    ) process_dir_entry(entry);
  } else {
    // process only the current directory, ignore child directories
    for (
      auto const &entry :
      fs::directory_iterator(cfg.search_path, dir_options)
    ) process_dir_entry(entry);
  }

  if (top_files.empty()) {
    return "No size and/or pattern matches";
  }

  for (size_t i = 0; i < top_files.size(); ++i) {
    auto const &file = top_files[i];

    char formatted_sz[20];
    util::format_file_size(file.m_size, formatted_sz, util::lengthof(formatted_sz));

    std::string const path = file.m_path.string();

    char const *const path_rel_to_search_dir =
      path.c_str() + cfg.search_path.size() + 1;

    out_ss
      << (i + 1) << ". "
      << '(' << formatted_sz << ") "
      << path_rel_to_search_dir << '\n';
  }

  std::string out = out_ss.str();

  if (!cfg.out_path.empty()) {
    std::fstream file;
    try {
      file = util::open_file(cfg.out_path.c_str(), std::ios::out);
    } catch (std::exception const &except) {
      out_ss << except.what() << '\n';
      return out_ss.str();
    }

    file
      << "top " << cfg.top_n << " largest files\n"
      << "in size range [" << cfg.min_size << ", " << cfg.max_size << "] bytes\n"
      << "in directory " << cfg.search_path;

    if (cfg.recurse) {
      file << " and child directories";
    }

    if (true) {
      file << '\n' << "matching regex /^" << cfg.pattern << "$/\n";
    }

    file
      << "----------\n"
      << out;
  }

  return std::move(out);
}
