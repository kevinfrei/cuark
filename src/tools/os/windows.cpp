#include <filesystem>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "file_tools.hpp"
#include "os.hpp"

namespace os {

bool is_hidden_file_os(const std::filesystem::path& path) {
  // On Windows, check if the file is hidden using the attributes
  DWORD attributes = GetFileAttributesW(path.c_str());
  return (attributes != INVALID_FILE_ATTRIBUTES) &&
         (attributes & FILE_ATTRIBUTE_HIDDEN);
}

} // namespace os

namespace files {

void root_iterator::populate_roots(bool include_empty) {
  // TODO: Only populate roots that actually exist?
  // TODO: Maybe only test existence on demand?
  // TODO: Probably skip A & B drives by default, or at least put them at the
  // end?
  DWORD mask = GetLogicalDrives();
  for (int i = 0; i < 26; ++i) {
    if (mask & (1 << i)) {
      char drive[] = {static_cast<char>('A' + i), ':', '\\', '\0'};
      m_roots.emplace_back(drive);
    }
  }
}

} // namespace files
