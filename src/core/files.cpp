#include <filesystem>
#include <fstream>
#include <iostream>

#include <crow/http_response.h>
#include <crow/logging.h>
#include <portable-file-dialogs.h>

#include "CommonTypes.hpp"
#include "tools.hpp"

#include "files.hpp"

namespace fs = std::filesystem;

namespace files {

fs::path program_location;
fs::path web_dir;

void set_program_location(const char* argv0) {
  if (argv0 == nullptr || argv0[0] == '\0') {
    CROW_LOG_ERROR << "argv[0] is empty, falling back to just 'cuark'";
    program_location = fs::canonical(fs::current_path() / "cuark");
    return;
  }
  fs::path potential_location(argv0);
  if (potential_location.is_absolute()) {
    program_location = fs::canonical(potential_location);
  } else if (potential_location.is_relative()) {
    program_location = fs::canonical(potential_location);
  } else {
    program_location = fs::canonical(fs::current_path() / argv0);
  }
}

fs::path get_web_dir() {
  if (web_dir.empty()) {
    fs::path cur = program_location;
    while (!fs::exists(cur / "web" / "index.html")) {
      cur = cur.parent_path();
    }
    web_dir = cur / "web";
  }
  return web_dir;
}

fs::path get_app_name() {
  return program_location.stem();
}

std::string path_to_mime_type(const fs::path& file_path) {
  const std::string extension = file_path.extension().generic_string();
  if (extension == ".txt") {
    return "text/plain";
  } else if (extension == ".css" || extension == ".scss") {
    return "text/css";
  } else if (extension == ".html" || extension == ".htm") {
    return "text/html";
  } else if (extension == ".js") {
    return "text/javascript";
  } else if (extension == ".gif") {
    return "image/gif";
  } else if (extension == ".jpg") {
    return "image/jpeg";
  } else if (extension == ".png") {
    return "image/png";
  } else if (extension == ".svg") {
    return "image/svg+xml";
  } else if (extension == ".flac") {
    return "audio/flac";
  } else if (extension == ".m4a") {
    return "audio/m4a";
  } else if (extension == ".mp3") {
    return "audio/mp3";
  } else if (extension == ".map") {
    return "application/json";
  } else if (extension == ".json") {
    return "application/json";
  }
  CROW_LOG_ERROR << "unknown: " << extension << ", reverting to text/html";
  return "text/html";
}

fs::path file_name_encode(std::string_view filename);
std::optional<std::string> file_name_decode(std::string_view filename);

/*
std::string LoadFileWithMimeType(const fs::path& path,
                                 std::string& mime_type) {
  std::ifstream input_file(path, std::ifstream::binary);

  // Get file size
  input_file.seekg(0, input_file.end);
  size_t file_size = input_file.tellg();
  input_file.seekg(0, input_file.beg);

  std::string ret;
  ret.resize(file_size);
  input_file.read(ret.data(), file_size);
  input_file.close();

  mime_type = ExtensionToMimeType(path.extension().string());

  return ret;
}
*/

// Mac and Linux can't have colons or slashes in filenames.
// Windows can't have a whole range of characters in filenames,
// *plus* names can't *end* in a period or space.
// Windows also doesn't allow filenames to have control characters
// (And mac/linux shouldn't either, but they don't enforce it).
// In addition, Windows, Mac, and Linux all *may* have case-insensitive
// filesystems, so we need to encode filenames in a way that keeps all
// that in mind.
fs::path file_name_encode(std::string_view filename) {
  // Encode the string provided as a valid, unique filename, which can
  // be re-translated back to it's original value.
  std::ostringstream oss;
  // The idea is to replace invalid characters with _X values,
  // and encode a case-flip as _-, so that we can reconstruct
  // the original filename later.
  bool is_upper = true; // We start in uppercase mode
  for (char c : filename) {
    switch (c) {
      case '/':
        oss << "_s"; // Slash
        break;
      case '\\':
        oss << "_b"; // Backslash
        break;
      case ':':
        oss << "_c"; // Colon
        break;
      case '*':
        oss << "_a"; // Asterisk
        break;
      case '?':
        oss << "_q"; // Question mark
        break;
      case '"':
        oss << "_d"; // Double quote
        break;
      case '<':
        oss << "_l"; // Less than
        break;
      case '>':
        oss << "_g"; // Greater than
        break;
      case '|':
        oss << "_p"; // Pipe
        break;
      case '.':
        oss << "_x"; // periods/eXtensions separator
        break;
      case ' ':
        oss << "__"; // space
        break;
      case '_':
        oss << "_u"; // underscore
        break;
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
        oss << "_n"
            << static_cast<char>(c + '0'); // Encode values 0-9 as _n0 to _n9
        break;
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
      case 21:
      case 22:
      case 23:
      case 24:
      case 25:
      case 26:
      case 27:
      case 28:
      case 29:
      case 30:
      case 31:
        oss << "_n"
            << static_cast<char>(c + 'a' - 10); // values 10-31 as _na to _nu
        break;
      default:
        if (!isalpha(c))
          oss << c; // Keep non-alphabetic characters as is
        else {
          if (!!isupper(c) != is_upper) {
            oss << "_-";
            is_upper = !is_upper; // Toggle case mode
          }
          oss << c; // Don't bother case-converting
        }
    }
  }
  return oss.str();
}

std::optional<std::string> file_name_decode(std::string_view filename) {
  // Decode the encoded filename back to its original form.
  std::ostringstream oss;
  bool is_upper = true; // We start in uppercase mode
  bool post_underscore = false; // Flag to handle post-underscore case toggling
  bool control_char = false; // Flag to handle control characters
  for (char c : filename) {
    if (post_underscore) {
      // If we are in post-underscore mode, we expect a special character
      switch (tolower(c)) {
        case 's':
          oss << '/';
          break;
        case 'b':
          oss << '\\';
          break;
        case 'c':
          oss << ':';
          break;
        case 'a':
          oss << '*';
          break;
        case 'q':
          oss << '?';
          break;
        case 'd':
          oss << '"';
          break;
        case 'l':
          oss << '<';
          break;
        case 'g':
          oss << '>';
          break;
        case 'p':
          oss << '|';
          break;
        case '_':
          oss << ' ';
          break;
        case 'x':
          oss << '.';
          break;
        case 'u':
          oss << '_';
          break;
        case '-':
          is_upper = !is_upper; // Toggle case mode
          break;
        case 'n':
          control_char = true; // Flag for control characters
          break;
        default:
          return std::nullopt; // Invalid character after underscore
      }
      post_underscore = false; // Reset the flag after processing
    } else if (c == '_') {
      post_underscore = true; // Set the flag for next character
    } else if (control_char) {
      // If we are in control character mode, we expect a digit
      if (std::isdigit(c)) {
        oss << static_cast<char>(c - '0'); // Convert to character
      } else if (std::isalpha(c)) {
        char l = static_cast<char>(tolower(c) - 'a' + 10);
        if (l > 31) {
          return std::nullopt;
        }
        oss << l;
      } else {
        return std::nullopt; // Invalid control character
      }
      control_char = false; // Reset the flag after processing
    } else {
      oss << static_cast<char>(is_upper ? toupper(c)
                                        : tolower(c)); // Convert to lowercase
    }
  }
  if (post_underscore || control_char) {
    return std::nullopt; // Incomplete encoding
  } else {
    return oss.str();
  }
}

std::optional<std::string> read_file(fs::path file_path) {
  constexpr size_t read_size = 4096;
  std::ifstream stream(file_path);
  stream.exceptions(std::ios_base::badbit);

  if (!stream) {
    return std::nullopt;
  }

  std::string out;
  std::string buf(read_size, '\0');
  while (stream.read(&buf[0], read_size))
    out.append(buf, 0, stream.gcount());

  out.append(buf, 0, stream.gcount());
  return out;
}

template <typename T>
void show_opt(std::string_view name, const std::optional<T>& opt) {
  if (opt) {
    CROW_LOG_INFO << "  " << name << ": " << *opt;
  } else {
    CROW_LOG_INFO << "  " << name << ": <none>";
  }
}

std::string new_folder_picker(
    const std::optional<Shared::OpenDialogOptions>& options) {
  if (options) {
    CROW_LOG_INFO << "Options: ";
    show_opt("folder", options->folder);
    show_opt("title", options->title);
    show_opt("defaultPath", options->defaultPath);
    show_opt("buttonLabel", options->buttonLabel);
    show_opt("multiSelections", options->multiSelections);
    if (options->filters) {
      CROW_LOG_INFO << "  filters: ";
      for (const auto& filter : *options->filters) {
        CROW_LOG_INFO << "    name: " << filter.name;
        CROW_LOG_INFO << "    extensions: ";
        for (const auto& ext : filter.extensions) {
          CROW_LOG_INFO << "      " << ext;
        }
      }
    }
  }
  auto result =
      pfd::select_folder("Select a folder", "", pfd::opt::none).result();
  return result;
}

void folder_picker(crow::response& resp, std::string_view data) {
  // TODO: Allow data to specify a title, default path or a platform path.
  // Use the sago::platform_folders thing, as it's started working in Conan
  // with 4.3.0
  CROW_LOG_INFO << "Folder picker called with data: " << data;
  auto options = from_json<Shared::OpenDialogOptions>(
      crow::json::load(data.data(), data.size()));
  if (options) {
    CROW_LOG_INFO << "Options: ";
    show_opt("folder", options->folder);
    show_opt("title", options->title);
    show_opt("defaultPath", options->defaultPath);
    show_opt("buttonLabel", options->buttonLabel);
    show_opt("multiSelections", options->multiSelections);
    if (options->filters) {
      CROW_LOG_INFO << "  filters: ";
      for (const auto& filter : *options->filters) {
        CROW_LOG_INFO << "    name: " << filter.name;
        CROW_LOG_INFO << "    extensions: ";
        for (const auto& ext : filter.extensions) {
          CROW_LOG_INFO << "      " << ext;
        }
      }
    }
  }
  auto result =
      pfd::select_folder("Select a folder", "", pfd::opt::none).result();
  if (result.empty()) {
    CROW_LOG_INFO << "Folder picker cancelled.";
    resp.code = 204; // No Content
    resp.body = "";
  } else {
    CROW_LOG_INFO << "Folder picker selected: " << result;
    resp.code = 200; // OK
    resp.set_header("Content-Type", "text/plain");
    auto path = (fs::path{result}).generic_string();
    resp.body = to_json(std::string{result}).dump();
  }
  /*
  auto json = crow::json::load(*maybe_value);
  CROW_LOG_INFO << "Writing data " << json << " for " << key << " to storage"
            ;
  std::ostringstream os;
  os << json;
  if (!config::write_to_storage(key, os.str())) {
    resp.code = 500; // Internal Server Error
  } else {
    resp.code = 200; // OK
  }
  */
}

} // namespace files
