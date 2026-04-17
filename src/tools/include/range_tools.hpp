#pragma once

#include <memory>
#include <ranges>
#include <string_view>

namespace views {
// 1. The ultra-fast conversion helper (O(1) pointer math)
inline constexpr auto to_string_view = std::views::transform([](auto&& rng) {
  auto b = std::ranges::begin(rng);
  auto e = std::ranges::end(rng);
  return std::string_view(
      std::to_address(b),
      static_cast<std::size_t>(std::to_address(e) - std::to_address(b)));
});

// 2. The 'Empty' filter
inline constexpr auto non_empty =
    std::views::filter([](std::string_view s) { return !s.empty(); });

// 3. A composite adaptor for paths or delimited lists
// Usage: path | split_sv('/')
inline auto split_string_view(char delim) {
  return std::views::split(delim) | to_string_view | non_empty;
}
} // namespace views
