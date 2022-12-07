#include <filesystem>
#include <functional>
#include <regex>
#include <vector>

#include <boost/program_options.hpp>

#include "action.hpp"
#include "exit.hpp"
#include "util.hpp"

using namespace std;
namespace fs = filesystem;

bool smallest(uintmax_t const currentFile, uintmax_t const lowestRankedFile) {
  return currentFile < lowestRankedFile;
}
bool largest(uintmax_t const currentFile, uintmax_t const lowestRankedFile) {
  return currentFile > lowestRankedFile;
}

action::Result action::sizerank(boost::program_options::variables_map const &options) {
  uintmax_t const top = [&options]() {
    if (options.count("top"))
      return options.at("top").as<uintmax_t>();
    else
      return static_cast<uintmax_t>(10);
  }();

  uintmax_t const minSize = [&options]() {
    if (options.count("minsize"))
      return options.at("minsize").as<uintmax_t>();
    else
      return static_cast<uintmax_t>(0);
  }();

  uintmax_t const maxSize = [&options]() {
    if (options.count("maxsize"))
      return options["maxsize"].as<uintmax_t>();
    else
      return UINTMAX_MAX;
  }();

  if (minSize > maxSize) {
    action::Result res(ExitCode::BAD_OPTION_VALUE);
    res.m_output << "fatal: option `minsize` must be <= `maxsize`";
    return res;
  }

  std::regex pattern;
  if (options.count("pattern")) {
    try {
      string const pattern_ = options.at("pattern").as<string>();
      pattern = (string("^") + pattern_ + "$");
    } catch (std::exception const &err) {
      action::Result res(ExitCode::BAD_OPTION_VALUE);
      res.m_output << "fatal: option `pattern` error - " << err.what();
      return res;
    }
  } else {
    pattern = ".*";
  }

  string const rootDir = options.count("dir")
    ? options.at("dir").as<string>()
    : fs::current_path().string();

  if (!fs::is_directory(rootDir)) {
    action::Result res(ExitCode::BAD_OPTION_VALUE);
    res.m_output << "fatal: option `dir` must be a directory";
    return res;
  }

  struct File {
    fs::path m_path;
    uintmax_t m_size;
  };

  size_t filesFound = 0;
  vector<File> topFiles{};
  topFiles.reserve(top + 1); // 1 extra for when we overflow

  auto const binaryInsert = [&topFiles](
    fs::path const &path,
    uintmax_t const size
  ) {
    if (topFiles.empty()) {
      topFiles.emplace_back(path, size);
      return;
    }

    int64_t first = 0;
    int64_t last = static_cast<uint64_t>(topFiles.size() - 1);

    while (last - first > 1) {
      int64_t const middle = (first + last) / 2;
      if (size < topFiles[middle].m_size)
        first = middle;
      else
        last = middle;
    }

    vector<File>::const_iterator const insertLoc = [&]() {
      if (size > topFiles[first].m_size)
        return topFiles.cbegin() + first;
      else if (size < topFiles[last].m_size)
        return topFiles.cbegin() + last + 1;
      else
        return topFiles.cbegin() + last;
    }();

    topFiles.emplace(insertLoc, path, size);
  };

  auto const processDirEntry = [&](fs::directory_entry const &entry) {
    if (entry.is_directory())
      return;

    error_code ec{};
    uintmax_t const size = fs::file_size(entry, ec);
    if (ec)
      return;

    ++filesFound;

    // quickest check, do it first
    if (size < minSize || size > maxSize)
      return;

    // second quickest check, do it second
    {
      bool const topFilesIsFull = topFiles.size() == top;
      if (topFilesIsFull) {
        uintmax_t const lowestRankedSize = topFiles[topFiles.size() - 1].m_size;
        if (size < lowestRankedSize)
          return;
      }
    }

    // slowest check, do it last
    {
      bool const matchesPattern =
        regex_match(entry.path().filename().string(), pattern);
      if (!matchesPattern)
        return;
    }

    binaryInsert(entry, size);
    if (topFiles.size() > top) {
      topFiles.pop_back();
    }
  };

  fs::directory_options dirOptions =
    fs::directory_options::skip_permission_denied;
  if (options.count("followdirsymlinks"))
    dirOptions |= fs::directory_options::follow_directory_symlink;

  // find top files
  if (options.count("recursive")) {
    // process each directory entry, and any subdirectories
    for (
      auto const &entry :
      fs::recursive_directory_iterator(rootDir, dirOptions)
    ) processDirEntry(entry);
  } else {
    // process only the current directory, ignore subdirectories
    for (
      auto const &entry :
      fs::directory_iterator(rootDir, dirOptions)
    ) processDirEntry(entry);
  }

  action::Result res(ExitCode::SUCCESS);

  if (topFiles.empty()) {
    res.m_output << "No matches";
  } else {
    for (size_t i = 0; i < topFiles.size(); ++i) {
      auto const &file = topFiles[i];

      char formattedSize[20];
      util::format_file_size(
        file.m_size,
        formattedSize,
        util::lengthof(formattedSize)
      );

      string const path = file.m_path.string();

      char const *const pathRelativeToRootDir =
        path.c_str() + rootDir.length() + 1;

      res.m_output
        << (i + 1) << ". "
        << '(' << formattedSize << ") "
        << pathRelativeToRootDir << '\n';
    }
  }

  if (options.count("out")) {
    string const pathname = options.at("out").as<string>();
    fstream file = util::open_file(pathname.c_str(), ios::out);
    file
      << "top " << top << " largest files\n"
      << "in range [" << minSize << ", " << maxSize << "] bytes\n"
      << "in directory \"" << rootDir << '"';
    if (options.count("recursive")) {
      file << " and all subdirectories";
    }
    file
      << "\nmatching regex /^" <<
        (options.count("pattern") ?
          options.at("pattern").as<string>()
          : ".*") << "$/\n"
      << "----------\n" << res.m_output.str();
  }

  return res;
}
