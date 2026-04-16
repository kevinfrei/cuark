#include <filesystem>
#include <windows.h>

#include "os.hpp"

namespace os {

bool is_hidden_file_os(const std::filesystem::path& path) {
  // On Windows, check if the file is hidden using the attributes
  DWORD attributes = GetFileAttributesW(path.c_str());
  return (attributes != INVALID_FILE_ATTRIBUTES) &&
         (attributes & FILE_ATTRIBUTE_HIDDEN);
}

} // namespace os
