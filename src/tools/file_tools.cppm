module;

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

export module tools.file;

import tools.text;
import os.tools.file;

namespace fs = std::filesystem;

export {

  namespace file {

  // TODO: Figure out the right thing to do for this one, that lives in
  // os/<platform>.cppm
  std::string get_home_dir() {
    return os::tools::file::get_home_dir();
  }

  bool is_hidden_file(const std::filesystem::path& path) {
    // Check if the file is hidden by checking if it starts with a dot
    if (path.filename().string().starts_with('.')) {
      return true;
    }
    if (!std::filesystem::exists(path)) {
      return false; // File does not exist
    }
    return os::tools::file::is_hidden_file_os(path);
  }

  void lowercase_extension(fs::path& p) {
    if (!p.has_extension())
      return;

    // Use the native string type (std::string or std::wstring)
    auto native_ext = p.extension().native();
    std::basic_string_view<fs::path::value_type> view(native_ext);

    // 1. Check if any characters actually need lowering
    bool needs_change = std::any_of(view.begin(), view.end(), [](auto c) {
      if constexpr (std::is_same_v<decltype(c), wchar_t>) {
        return std::iswupper(c);
      } else {
        return std::isupper(static_cast<unsigned char>(c));
      }
    });

    if (!needs_change)
      return;

    // 2. Perform transformation using the native character type
    std::basic_string<fs::path::value_type> lowered_ext;
    lowered_ext.reserve(view.size());

    for (auto c : view) {
      lowered_ext += text::toggle_lower(c);
    }

    p.replace_extension(lowered_ext);
  }

  void foreach_line_in_file(const fs::path& filePath,
                            const std::function<void(const std::string&)>& fn) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
      std::cerr << "Failed to open file: " << filePath << "\n";
      return;
    }
    std::string line;
    while (std::getline(file, line)) {
      fn(line);
    }
  }

  class root_iterator {

   private:
    size_t m_index;
    std::vector<std::filesystem::path> m_roots;

    // Helper to check if we've reached the end of the roots
    bool at_end() const {
      return m_index >= m_roots.size();
    }

    void populate_roots(bool include_empty) {
      m_roots = os::tools::file::populate_roots_os(include_empty);
    }

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

  } // namespace file

} // export