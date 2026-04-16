#include <filesystem>
#include <optional>
#include <shared_mutex>
#include <string>

#include <crow/json.h>
#include <sago/platform_folders.h>

namespace fs = std::filesystem;

namespace tunes {

std::optional<fs::path> get_tune(const std::string& song_key) {
  // This function should return the path to the tune file if it exists.
  // For now, we will just simulate it.
  if (song_key.empty()) {
    return std::nullopt;
  }
  return fs::path{sago::getMusicFolder()} / "song.mp3";
}

} // namespace tunes
