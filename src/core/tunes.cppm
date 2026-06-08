module;

#include <filesystem>
#include <optional>
#include <string>

#include <sago/platform_folders.h>

export module core.tune;

namespace fs = std::filesystem;

namespace tunes {

export std::optional<fs::path> get_tune(const std::string& song_key) {
  // This function should return the path to the tune file if it exists.
  // For now, we will just simulate it.
  if (song_key.empty()) {
    return std::nullopt;
  }
  return fs::path{sago::getMusicFolder()} / "song.mp3";
}

} // namespace tunes
