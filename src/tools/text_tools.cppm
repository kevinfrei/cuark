module;

#include <algorithm>
#include <cctype>
#include <charconv>
#include <codecvt>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>

#include <crow/json.h>

export module tools.text;

import ts_cpp_idl.crow_support;

constexpr char32_t COMBINING_GRAVE = 0x0300;
constexpr char32_t COMBINING_ACUTE = 0x0301;
constexpr char32_t COMBINING_CIRCUMFLEX = 0x0302;
constexpr char32_t COMBINING_TILDE = 0x0303;
constexpr char32_t COMBINING_DIAERESIS = 0x0308;
constexpr char32_t COMBINING_RING_ABOVE = 0x030a;
constexpr char32_t COMBINING_CEDILLA = 0x0327;
constexpr char32_t COMBINING_SLASH = 0x0337;

// This puts the codepoint, as UTF8, into result. It will split diacritics into
// separate combining characters.
void handle_codepoint(std::string& result, char32_t codepoint);

// This will uncombine common Latin letters with diacritics into base letters,
// returning true if it did so, false if it didn't.
// If it returns false, the caller should handle the codepoint normally.
bool uncombine_latin(std::string& result, char32_t codepoint);

bool is_onebyte(unsigned char c) {
  return c <= 0x7f;
}

bool is_twobyte_prefix(unsigned char c) {
  return (c & 0xe0) == 0xc0;
}

bool is_twobyte_continuation(std::string_view input,
                             std::string_view::size_type i,
                             unsigned char* c2) {
  if (i + 1 >= input.size())
    return false;
  *c2 = static_cast<unsigned char>(input[i + 1]);
  return (*c2 & 0xc0) == 0x80;
}

bool is_threebyte_prefix(unsigned char c) {
  return (c & 0xf0) == 0xe0;
}

bool is_threebyte_continuation(std::string_view input,
                               std::string_view::size_type i,
                               unsigned char* c2,
                               unsigned char* c3) {
  if (i + 2 >= input.size())
    return false;
  *c2 = static_cast<unsigned char>(input[i + 1]);
  *c3 = static_cast<unsigned char>(input[i + 2]);
  return ((*c2 & 0xc0) == 0x80 && (*c3 & 0xc0) == 0x80);
}

bool is_fourbyte_prefix(unsigned char c) {
  return (c & 0xf8) == 0xf0;
}

bool is_fourbyte_continuation(std::string_view input,
                              std::string_view::size_type i,
                              unsigned char* c2,
                              unsigned char* c3,
                              unsigned char* c4) {
  if (i + 3 >= input.size())
    return false;
  *c2 = static_cast<unsigned char>(input[i + 1]);
  *c3 = static_cast<unsigned char>(input[i + 2]);
  *c4 = static_cast<unsigned char>(input[i + 3]);
  return ((*c2 & 0xc0) == 0x80 && (*c3 & 0xc0) == 0x80 && (*c4 & 0xc0) == 0x80);
}

void handle_codepoint(std::string& result, char32_t codepoint) {
  // Convert the 'unified' diacritics to suffixed modifiers.
  if (!uncombine_latin(result, codepoint)) {
    if (codepoint <= 0x7f) {
      result.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7ff) {
      result.push_back(static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
      result.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
    } else if (codepoint <= 0xffff) {
      result.push_back(static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
      result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
    } else {
      result.push_back(static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
      result.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
    }
  }
}

bool uncombine_latin(std::string& result, char32_t codepoint) {
  switch (codepoint) {
    case 0xc0:
      // Latin Capital Letter A with Grave
      handle_codepoint(result, 'A');
      handle_codepoint(result, COMBINING_GRAVE);
      break;
    case 0xc1:
      // Latin Capital Letter A with Acute
      handle_codepoint(result, 'A');
      handle_codepoint(result, COMBINING_ACUTE);
      break;
    case 0xc2:
      handle_codepoint(result, 'A');
      handle_codepoint(result, COMBINING_CIRCUMFLEX);
      break;
    case 0xc3:
      handle_codepoint(result, 'A');
      handle_codepoint(result, COMBINING_TILDE);
      break;
    case 0xc4:
      handle_codepoint(result, 'A');
      handle_codepoint(result, COMBINING_DIAERESIS);
      break;
    case 0xc5:
      handle_codepoint(result, 'A');
      handle_codepoint(result, COMBINING_RING_ABOVE);
      break;
    case 0xc8:
      handle_codepoint(result, 'E');
      handle_codepoint(result, COMBINING_GRAVE);
      break;
    case 0xc9:
      handle_codepoint(result, 'E');
      handle_codepoint(result, COMBINING_ACUTE);
      break;
    case 0xca:
      handle_codepoint(result, 'E');
      handle_codepoint(result, COMBINING_CIRCUMFLEX);
      break;
    case 0xcb:
      handle_codepoint(result, 'E');
      handle_codepoint(result, COMBINING_DIAERESIS);
      break;
    case 0xcc:
      handle_codepoint(result, 'I');
      handle_codepoint(result, COMBINING_GRAVE);
      break;
    case 0xcd:
      handle_codepoint(result, 'I');
      handle_codepoint(result, COMBINING_ACUTE);
      break;
    case 0xce:
      handle_codepoint(result, 'I');
      handle_codepoint(result, COMBINING_CIRCUMFLEX);
      break;
    case 0xcf:
      handle_codepoint(result, 'I');
      handle_codepoint(result, COMBINING_DIAERESIS);
      break;
    case 0xe0:
      handle_codepoint(result, 'a');
      handle_codepoint(result, COMBINING_GRAVE);
      break;
    case 0xe1:
      handle_codepoint(result, 'a');
      handle_codepoint(result, COMBINING_ACUTE);
      break;
    case 0xe2:
      handle_codepoint(result, 'a');
      handle_codepoint(result, COMBINING_CIRCUMFLEX);
      break;
    case 0xe3:
      handle_codepoint(result, 'a');
      handle_codepoint(result, COMBINING_TILDE);
      break;
    case 0xe4:
      handle_codepoint(result, 'a');
      handle_codepoint(result, COMBINING_DIAERESIS);
      break;
    case 0xe5:
      handle_codepoint(result, 'a');
      handle_codepoint(result, COMBINING_RING_ABOVE);
      break;
    case 0xe8:
      handle_codepoint(result, 'e');
      handle_codepoint(result, COMBINING_GRAVE);
      break;
    case 0xe9:
      handle_codepoint(result, 'e');
      handle_codepoint(result, COMBINING_ACUTE);
      break;
    case 0xea:
      handle_codepoint(result, 'e');
      handle_codepoint(result, COMBINING_CIRCUMFLEX);
      break;
    case 0xeb:
      handle_codepoint(result, 'e');
      handle_codepoint(result, COMBINING_DIAERESIS);
      break;
    case 0xec:
      handle_codepoint(result, 'i');
      handle_codepoint(result, COMBINING_GRAVE);
      break;
    case 0xed:
      handle_codepoint(result, 'i');
      handle_codepoint(result, COMBINING_ACUTE);
      break;
    case 0xee:
      handle_codepoint(result, 'i');
      handle_codepoint(result, COMBINING_CIRCUMFLEX);
      break;
    case 0xef:
      handle_codepoint(result, 'i');
      handle_codepoint(result, COMBINING_DIAERESIS);
      break;
    case 0xd2:
      handle_codepoint(result, 'O');
      handle_codepoint(result, COMBINING_GRAVE);
      break;
    case 0xd3:
      handle_codepoint(result, 'O');
      handle_codepoint(result, COMBINING_ACUTE);
      break;
    case 0xd4:
      handle_codepoint(result, 'O');
      handle_codepoint(result, COMBINING_CIRCUMFLEX);
      break;
    case 0xd5:
      handle_codepoint(result, 'O');
      handle_codepoint(result, COMBINING_TILDE);
      break;
    case 0xd6:
      handle_codepoint(result, 'O');
      handle_codepoint(result, COMBINING_DIAERESIS);
      break;
    case 0xd8:
      handle_codepoint(result, 'O');
      handle_codepoint(result, COMBINING_SLASH);
      break;
    case 0xf2:
      handle_codepoint(result, 'o');
      handle_codepoint(result, COMBINING_GRAVE);
      break;
    case 0xf3:
      handle_codepoint(result, 'o');
      handle_codepoint(result, COMBINING_ACUTE);
      break;
    case 0xf4:
      handle_codepoint(result, 'o');
      handle_codepoint(result, COMBINING_CIRCUMFLEX);
      break;
    case 0xf5:
      handle_codepoint(result, 'o');
      handle_codepoint(result, COMBINING_TILDE);
      break;
    case 0xf6:
      handle_codepoint(result, 'o');
      handle_codepoint(result, COMBINING_DIAERESIS);
      break;
    case 0xf8:
      handle_codepoint(result, 'o');
      handle_codepoint(result, COMBINING_SLASH);
      break;
    case 0xd9:
      handle_codepoint(result, 'U');
      handle_codepoint(result, COMBINING_GRAVE);
      break;
    case 0xda:
      handle_codepoint(result, 'U');
      handle_codepoint(result, COMBINING_ACUTE);
      break;
    case 0xdb:
      handle_codepoint(result, 'U');
      handle_codepoint(result, COMBINING_CIRCUMFLEX);
      break;
    case 0xdc:
      handle_codepoint(result, 'U');
      handle_codepoint(result, COMBINING_DIAERESIS);
      break;
    case 0xf9:
      handle_codepoint(result, 'u');
      handle_codepoint(result, COMBINING_GRAVE);
      break;
    case 0xfa:
      handle_codepoint(result, 'u');
      handle_codepoint(result, COMBINING_ACUTE);
      break;
    case 0xfb:
      handle_codepoint(result, 'u');
      handle_codepoint(result, COMBINING_CIRCUMFLEX);
      break;
    case 0xfc:
      handle_codepoint(result, 'u');
      handle_codepoint(result, COMBINING_DIAERESIS);
      break;
    case 0xc7:
      handle_codepoint(result, 'C');
      handle_codepoint(result, COMBINING_CEDILLA);
      break;
    case 0xe7:
      handle_codepoint(result, 'c');
      handle_codepoint(result, COMBINING_CEDILLA);
      break;
    case 0xd1:
      handle_codepoint(result, 'N');
      handle_codepoint(result, COMBINING_TILDE);
      break;
    case 0xf1:
      handle_codepoint(result, 'n');
      handle_codepoint(result, COMBINING_TILDE);
      break;
    default:
      return false; // Not a combined character we know about
  }

  return true;
}

export {

  namespace text {

  // Helper to handle both char and wchar_t lowering
  template <typename T>
  T toggle_lower(T c) {
    if constexpr (std::is_same_v<T, wchar_t>) {
      return std::towlower(c);
    } else {
      return static_cast<T>(std::tolower(static_cast<unsigned char>(c)));
    }
  }

  template <typename T>
  std::enable_if_t<!is_enum_class_v<T>, std::optional<T>> to_integer(
      std::string_view sv) {
    T value;
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
    if (ec == std::errc{} && ptr == sv.data() + sv.size()) {
      return value;
    }
    return std::nullopt;
  }

  template <typename T>
  inline std::enable_if_t<is_enum_class_v<T>, std::optional<T>> to_integer(
      std::string_view value) {
    auto res = to_integer<std::underlying_type_t<T>>(value);
    if (res) {
      return static_cast<T>(*res);
    } else {
      return std::nullopt;
    }
  }

  template <typename T>
  T from_string(std::string_view s) {
    if constexpr (std::is_same_v<T, std::string_view>) {
      return s;
    } else if constexpr (std::is_same_v<T, std::string>) {
      // Handle std::string types
      return std::string(s);
    } else if constexpr (std::is_same_v<T, bool>) {
      // Handle boolean types: This is weak, but I don't really care too much,
      // right now...
      return (s == "true");
    } else if constexpr (std::is_floating_point_v<T>) {
      // macos/Apple clang doesn't support std::from_chars for floating-point
      // types, so we have to fall back to strtof/d, which is slower but more
      // widely supported. We also need to check that the entire string was
      // consumed, to avoid cases where "abc" would be converted to 0.0 without
      // error.
      char* end;
      T res = (std::is_same_v<T, float>) ? std::strtof(s.data(), &end)
                                         : std::strtod(s.data(), &end);
      return (end == s.data() + s.size()) ? res : T{};
    } else if constexpr (std::is_integral_v<T> && sizeof(T) < 8) {
      // Handle numeric types (int, char, short, etc...), but we have to handle
      // 64 bit integers separately, because on the Javascript side they have to
      // be BigInts, so they get encoded differently.
      T val{};
      // std::from_chars is strict and fast
      auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);

      // If conversion fails (e.g. "abc" -> int), return 0
      if (ec == std::errc{}) {
        return static_cast<T>(val);
      }
    } else {
      // For other types, we unpickle it using the from_json framework from
      // crow-idl
      auto res = from_json<T>(crow::json::load(s.data(), s.size()));
      if (res.has_value()) {
        return res.value();
      }
    }
    return T{};
  }

  template <typename T>
  T from_string(const std::string& s) {
    if constexpr (std::is_same_v<T, std::string>) {
      // Handle std::string types
      return s;
    }
    return from_string<T>(std::string_view{s});
  }

  template <typename T>
  T from_string(const char* s) {
    return from_string<T>(std::string_view{s});
  }

  std::string lowercase(std::string_view str);
  bool iequals(std::string_view lhs, std::string_view rhs);

// Suppress deprecation warnings for legacy support
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#else
#pragma warning(push)
#pragma warning(disable : 4996) // MSVC
#endif

  // Usage Examples:
  // std::string utf8 = convert_string<char, wchar_t>(wide_str); // wstring ->
  // string std::wstring wide = convert_string<wchar_t, char>(utf8_str); //
  // string
  // -> wstring
  template <typename TargetChar, typename SourceChar>
  std::basic_string<TargetChar> convert_string(
      const std::basic_string<SourceChar>& input) {
    if (input.empty())
      return {};

    if constexpr (std::is_same_v<TargetChar, SourceChar>) {
      return input;
    }
    // Define conversion types based on source/target
    using Cvtptr = std::conditional_t<
        std::is_same_v<SourceChar, char> && std::is_same_v<TargetChar, wchar_t>,
        std::codecvt_utf8_utf16<wchar_t>, // char (UTF-8) -> wchar_t
        std::conditional_t<std::is_same_v<SourceChar, wchar_t> &&
                               std::is_same_v<TargetChar, char>,
                           std::codecvt_utf8_utf16<wchar_t>, // wchar_t -> char
                                                             // (UTF-8)
                           void // Add other combinations if needed
                           >>;

    static_assert(!std::is_same_v<Cvtptr, void>,
                  "Unsupported conversion combination");

    std::wstring_convert<Cvtptr, SourceChar> converter;

    if constexpr (std::is_same_v<SourceChar, char>) {
      // Converting FROM narrow string
      auto wide_intermediate = converter.from_bytes(input);
      return std::basic_string<TargetChar>(wide_intermediate.begin(),
                                           wide_intermediate.end());
    } else {
      // Converting FROM wide string
      return converter.to_bytes(input);
    }
  }

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#else
#pragma warning(pop)
#endif

  // Normalize a string to diacritic post-modifiers form. This is a very
  // simplistic implementation that only handles a few common cases. A full
  // implementation would use ICU or similar library.
  std::string normalize_latin_to_utf8(std::string_view input) {
    std::string result;
    // Convert the code-page encodings to UTF-8 encodings, in the most
    // simplistic way possible.
    for (auto chr : input) {
      unsigned char c = static_cast<unsigned char>(chr);
      if (!uncombine_latin(result, c)) {
        handle_codepoint(result, c);
      }
    }
    return result;
  }

  // This is the most likely entrypoint for callers. It will take a string that
  // may be in UTF-8 or Latin-1, and return a normalized UTF-8 string with
  // diacritics as combining characters.
  std::string normalize_utf8_or_latin(std::string_view input) {
    std::string result;
    for (size_t i = 0; i < input.size(); i++) {
      unsigned char c = static_cast<unsigned char>(input[i]);
      char32_t codepoint = 0;
      if (is_onebyte(c)) {
        codepoint = c;
      } else if (is_twobyte_prefix(c)) {
        // Two-byte sequence
        unsigned char c2;
        if (!is_twobyte_continuation(input, i, &c2)) {
          // Invalid continuation byte, or too short. Treat it like a single
          // byte
          codepoint = c;
        } else {
          // High 5 bits in byte 0, next 6 bits in byte 1
          codepoint = ((c & 0x1f) << 6) | (c2 & 0x3f);
          i++; // Consumed an extra byte
        }
      } else if (is_threebyte_prefix(c)) {
        // Three-byte sequence
        unsigned char c2, c3;
        if (!is_threebyte_continuation(input, i + 1, &c2, &c3)) {
          // Invalid continuation bytes, just copy the first byte
          codepoint = c;
        } else {
          // High 4 bits in byte 0, next 6 bits in byte 1, last 6 bits in byte 2
          codepoint = ((c & 0x0f) << 12) | ((c2 & 0x3f) << 6) | (c3 & 0x3f);
          i += 2; // Consumed two extra bytes
        }
      } else if (is_fourbyte_prefix(c)) {
        // Four-byte sequence
        unsigned char c2, c3, c4;
        if (!is_fourbyte_continuation(input, i, &c2, &c3, &c4)) {
          codepoint = c;
        } else {
          // High 3 bits in byte 0, next 6 bits in byte 1, next 6 bits in byte
          // 2, last 6 bits in byte 3
          codepoint = ((c & 0x07) << 18) | ((c2 & 0x3f) << 12) |
                      ((c3 & 0x3f) << 6) | (c4 & 0x3f);
          i += 3; // Consumed three extra bytes
        }
      } else {
        codepoint = c; // Invalid byte, just copy it
      }
      handle_codepoint(result, codepoint);
    }
    return result;
  }

  // This is the most likely entrypoint for callers. It will take a string that
  // may be in UTF-8 or Latin-1, and return a normalized UTF-8 string with
  // diacritics as combining characters.
  std::string normalize_utf8_or_latin(std::u8string_view input) {
    std::string_view sv(reinterpret_cast<const char*>(input.data()),
                        input.length());
    return normalize_utf8_or_latin(sv);
  }

  // Just lowercase an ASCII string
  std::string lowercase(std::string_view str) {
    std::string result;
    result.reserve(str.size());
    for (char c : str) {
      result += toggle_lower(c);
    }
    return result;
  }

  bool iequals(std::string_view lhs, std::string_view rhs) {
    return std::ranges::equal(lhs, rhs, [](unsigned char a, unsigned char b) {
      return std::tolower(a) == std::tolower(b);
    });
  }

  } // namespace text

} // export