module;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

export module os.tools.file;

import std;

namespace os::tools::file {

export bool is_hidden_file_os(const std::filesystem::path& path) {
  // On Windows, check if the file is hidden using the attributes
  DWORD attributes = GetFileAttributesW(path.c_str());
  return (attributes != INVALID_FILE_ATTRIBUTES) &&
         (attributes & FILE_ATTRIBUTE_HIDDEN);
}

export std::vector<std::filesystem::path> populate_roots_os(
    bool /*include_empty*/) {
  // TODO: Only populate roots that actually exist?
  // TODO: Maybe only test existence on demand?
  // TODO: Probably skip A & B drives by default, or at least put them at the
  // end?
  std::vector<std::filesystem::path> m_roots;
  DWORD mask = GetLogicalDrives();
  for (int i = 0; i < 26; ++i) {
    if (mask & (1 << i)) {
      char drive[] = {static_cast<char>('A' + i), ':', '\\', '\0'};
      m_roots.emplace_back(drive);
    }
  }
  return m_roots;
}

std::string get_env(const std::string& var) {
  DWORD bufferSize = GetEnvironmentVariableA(var.c_str(), nullptr, 0);
  if (bufferSize > 0) {
    char* buffer = new char[bufferSize];
    GetEnvironmentVariableA(var.c_str(), buffer, bufferSize);
    std::string res{buffer};
    delete[] buffer;
    return res;
  }
  return "";
}

export std::string get_home_dir() {
  auto userprofile = get_env("USERPROFILE");
  if (!userprofile.empty()) {
    return userprofile;
  }
  auto drive = get_env("HOMEDRIVE");
  auto path = get_env("HOMEPATH");
  if (drive.empty() || path.empty()) {
    return "";
  }
  return std::string(drive) + path;
}

} // namespace os::tools::file
