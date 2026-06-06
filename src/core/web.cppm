module;

#include <cctype>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <string_view>

#include <crow/http_response.h>
#include <webview.h>

export module core.web;

namespace web {

uint16_t port = 0;

export uint16_t get_random_port() {
  if (port == 0) {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 16383);
    port = 49152 + static_cast<uint16_t>(dist(rd)); // Use ports in the range
                                                    // 49152-65535
  }
  return port;
}

export void e404(crow::response& resp, const std::string& message) {
  CROW_LOG_ERROR << "Error 404: " << message;
  resp.code = 404;
  resp.body = message;
  resp.set_header("Content-Type", "text/plain");
}

export std::string url_encode(std::string_view value) {
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;

  for (auto i = value.begin(), n = value.end(); i != n; ++i) {
    std::string_view::value_type c = (*i);

    // Keep alphanumeric and other accepted characters intact
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      escaped << c;
      continue;
    }

    // Any other characters are percent-encoded
    escaped << std::uppercase;
    escaped << '%' << std::setw(2) << int((unsigned char)c);
    escaped << std::nouppercase;
  }

  return escaped.str();
}

bool is_valid_url_char(char c) {
  // Check if the character is a valid URL character
  if (std::isalnum(c))
    return true;
  switch (c) {
    case '$':
    case '-':
    case '_':
    case '.':
    case '+':
    case '!':
    case '*':
    case '\'':
    case '(':
    case ')':
      return true;
    default:
      return false;
  }
}

int hex_val(char c) {
  assert(std::isxdigit(c) && "hex_val called with non-hex character");
  // Convert a hex character to its integer value
  if (std::isdigit(c)) {
    return c - '0'; // '0' to '9'
  } else {
    return std::toupper(c) - 'A' + 10; // 'A' to 'F'
  }
}

export std::optional<std::string> url_decode(std::string_view value) {
  std::ostringstream unescaped;
  int hexPos = -1;
  int hexValue = 0;
  for (auto c : value) {
    if (hexPos == 0 || hexPos == 1) {
      // We are in a hex sequence
      if (std::isxdigit(c)) {
        hexValue = (hexValue << 4) | hex_val(c);
        if (hexPos == 1) {
          // We have both hex digits, convert to character}
          unescaped << static_cast<char>(hexValue);
          hexPos = -1; // Reset hex position
        } else {
          hexPos = 1; // Move to next hex digit
        }
      } else {
        return std::nullopt; // Invalid hex sequence
      }
    } else if (c == '%') {
      // Start of a hex sequence
      hexPos = 0;
    } else if (is_valid_url_char(c)) {
      // Normal character, just append it
      unescaped << c;
    } else {
      return std::nullopt; // Invalid character
    }
  }
  return unescaped.str();
}

export void open(const std::string& url) {
  // 3. Initialize Webview on the Main Thread
  webview::webview w(true, nullptr);
  w.set_title("Cuark Template");
  w.set_size(800, 600, WEBVIEW_HINT_NONE);
  w.navigate(url);
  w.run(); // This blocks until the window is closed
}

} // namespace web