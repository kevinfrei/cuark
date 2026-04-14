#pragma once

#include <cctype>
#include <charconv>
#include <cwctype>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

#include "json_pickling.hpp"

namespace text {

// Normalize a string to diacritic post-modifiers form. This is a very
// simplistic implementation that only handles a few common cases. A full
// implementation would use ICU or similar library.
std::string normalize_latin_to_utf8(std::string_view input);

// This is the most likely entrypoint for callers. It will take a string that
// may be in UTF-8 or Latin-1, and return a normalized UTF-8 string with
// diacritics as combining characters.
std::string normalize_utf8_or_latin(std::string_view input);
std::string normalize_utf8_or_latin(std::u8string_view input);

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
T from_string(const std::string& s) {
  if constexpr (std::is_same_v<T, std::string>) {
    // Handle std::string types
    return s;
  } else if constexpr (std::is_same_v<T, std::string_view>) {
    // Warning: This is only safe if the underlying string outlives the view!
    return std::string_view(s);
  } else if constexpr (std::is_arithmetic_v<T>) {
    // Handle numeric types (int, double, etc.)
    T val{};
    // std::from_chars is strict and fast
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);

    // If conversion fails (e.g. "abc" -> int), return 0
    if (ec == std::errc{}) {
      return val;
    }
  }
  return T{};
}

template <typename T>
T from_string(const std::string_view& s) {
  if constexpr (std::is_same_v<T, std::string_view>) {
    return s;
  } else if constexpr (std::is_same_v<T, std::string>) {
    // Handle std::string types
    return std::string(s);
  } else if constexpr (std::is_arithmetic_v<T>) {
    // Handle numeric types (int, double, etc.)
    T val{};
    // std::from_chars is strict and fast
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);

    // If conversion fails (e.g. "abc" -> int), return 0
    if (ec == std::errc{}) {
      return val;
    }
  }
  return T{};
}

template <typename T>
T from_string(const char* s) {
  return from_string<T>(std::string_view{s});
}

std::string lowercase(std::string_view str);
bool iequals(std::string_view lhs, std::string_view rhs);

} // namespace text
