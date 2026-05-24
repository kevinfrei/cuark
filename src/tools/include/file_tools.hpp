#pragma once

#include <cstddef>
#include <filesystem>
#include <functional>
#include <iterator>
#include <string>
#include <vector>

namespace files {

bool is_hidden_file(const std::filesystem::path& path);
void lowercase_extension(std::filesystem::path& p);
void foreach_line_in_file(const std::filesystem::path& filePath,
                          const std::function<void(const std::string&)>& fn);
std::string get_home_dir();

class root_iterator {

 private:
  size_t m_index;
  std::vector<std::filesystem::path> m_roots;

  // Helper to check if we've reached the end of the roots
  bool at_end() const {
    return m_index >= m_roots.size();
  }

  // OS-specific stuff lives in this thing
  void populate_roots(bool include_empty);

 public:
  using iterator_category = std::input_iterator_tag;
  using value_type = std::filesystem::path;
  using difference_type = std::ptrdiff_t;
  using pointer = const std::filesystem::path*;
  using reference = const std::filesystem::path&;

  // Begin iterator
  explicit root_iterator(bool include_empty) : m_index(0) {
    populate_roots(include_empty);
  }

  // End iterator
  root_iterator() : m_index(0) {}

  reference operator*() const {
    return m_roots[m_index];
  }

  pointer operator->() const {
    return &m_roots[m_index];
  }

  root_iterator& operator++() {
    if (!at_end()) {
      m_index++;
    }
    return *this;
  }

  bool operator==(const root_iterator& other) const {
    bool this_end = at_end(), other_end = other.at_end();
    if (this_end != other_end) {
      return false;
    }
    // if this_end is true, other_end is also true, because they're equal.
    return this_end || (m_index == other.m_index);
  }

  bool operator!=(const root_iterator& other) const {
    return !(*this == other);
  }
};

struct drive_range {
  root_iterator begin(bool include_empty = false) const {
    return root_iterator(include_empty);
  }
  root_iterator end() const {
    return root_iterator();
  }
};

} // namespace files
