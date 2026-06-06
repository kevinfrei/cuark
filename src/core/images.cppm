module;

#include <filesystem>
#include <string>

#include "files.hpp"

export module core.images;

import core.config;

namespace image {

export std::filesystem::path get_image_path(const std::string& query_path) {
  auto slash = query_path.find('/');
  if (slash != std::string::npos) {
    // With a slash, we can have 'album' or 'artist' followed by the key, and we
    // can return the corresponding image.
    std::string type = query_path.substr(0, slash);
    std::string key = query_path.substr(slash + 1);
    // If there's a slash, we should check if the first part is "album" or
    // "artist", and if so, we can return a path to the corresponding image.
    return std::filesystem::path(files::get_web_dir() / "img" /
                                 (type + ".svg"));
  }
  // For now, this is clearly bad & wrong, but I don't care right now.
  return std::filesystem::path(files::get_web_dir() / "img" / "icon.svg");
}

} // namespace image