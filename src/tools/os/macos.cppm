module;

#include <pwd.h>
#include <unistd.h>

export module os.tools.file;

import std;

import std;

namespace os::tools::file {

export bool is_hidden_file_os(const std::filesystem::path& /*path*/) {
  return false;
}

export std::vector<std::filesystem::path> populate_roots_os(bool) {
  std::vector<std::filesystem::path> m_roots;
  // Always include the root
  m_roots.emplace_back("/");

  // macOS: Scan /Volumes but skip the root shadow
  std::error_code ec;
  if (std::filesystem::exists("/Volumes", ec)) {
    for (const auto& entry :
         std::filesystem::directory_iterator("/Volumes", ec)) {
      // Check if this volume is effectively the same as /
      if (!std::filesystem::equivalent(entry.path(), "/", ec)) {
        m_roots.push_back(entry.path());
      }
    }
  }
  return m_roots;
}

export std::string get_home_dir() {
  const char* home = std::getenv("HOME");
  if (home != nullptr) {
    return home;
  }
  struct passwd* pwd = getpwuid(getuid());
  if (pwd != nullptr) {
    return pwd->pw_dir;
  }
  return ""; // Failed to retrieve
}

} // namespace os::tools::file