#include <filesystem>

#include "os.hpp"

namespace os {
bool is_hidden_file_os(const std::filesystem::path& path) {
  return false;
}
} // namespace os
