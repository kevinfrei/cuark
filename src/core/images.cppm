module;

// If I remove this, Clang 22.1.6 ICE's :(
#include <string>

export module core.images;

import std;

import core.config;
import core.file;

namespace fs = std::filesystem;
using std::string;

namespace image {

export fs::path get_image_path(const string& query_path) {
  auto slash = query_path.find('/');
  if (slash != string::npos) {
    // With a slash, we can have 'album' or 'artist' followed by the key, and we
    // can return the corresponding image.
    string type = query_path.substr(0, slash);
    string key = query_path.substr(slash + 1);
    // If there's a slash, we should check if the first part is "album" or
    // "artist", and if so, we can return a path to the corresponding image.
    return fs::path(files::get_web_dir() / "img" / (type + ".svg"));
  }
  // For now, this is clearly bad & wrong, but I don't care right now.
  return fs::path(files::get_web_dir() / "img" / "icon.svg");
}

} // namespace image