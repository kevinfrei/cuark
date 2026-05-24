#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <pwd.h>
#include <unistd.h>

#include "file_tools.hpp"
#include "os.hpp"

namespace os {
bool is_hidden_file_os(const std::filesystem::path& /*path*/) {
  return false;
}
} // namespace os

namespace files {

void root_iterator::populate_roots(bool) {
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
}

std::string get_home_dir() {
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

} // namespace files
