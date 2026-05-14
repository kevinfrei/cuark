#include <string>

#include <gtest/gtest.h>

#include "file_tools.hpp"
#include "hacks.hpp"
#include "text_tools.hpp"

// clang-format off
constexpr uint8_t latin_chars[] = {
  0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, // upper A
  0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, // lower a
  0xc8, 0xc9, 0xca, 0xcb, 0xe8, 0xe9, 0xea, 0xeb, // E's
  0xcc, 0xcd, 0xce, 0xcf, 0xec, 0xed, 0xee, 0xef, // I's
  0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd8, // upper O
  0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf8, // lower O
  0xd9, 0xda, 0xdb, 0xdc, 0xf9, 0xfa, 0xfb, 0xfc, // U's
  0xc7, 0xe7, 0xd1, 0xf1, 0 // C's, N's
};
// clang-format on

TEST(Text, BasicNormalization) {
  // const char *input =
  // u8"ÀÁÂÃÄÅàáâãäåÈÉÊËèéêëÌÍÎÏìíîïÒÓÔÕÖØòóôõöøÙÚÛÜùúûüÇçÑñÝýÿ";
  const char* input = reinterpret_cast<const char*>(&latin_chars[0]);
  // "ÀÁÂÃÄÅàáâãäåÈÉÊËèéêëÌÍÎÏìíîïÒÓÔÕÖØòóôõöøÙÚÛÜùúûüÇçÑñ");
  const char* expected = reinterpret_cast<const char*>(
      u8"A\u0300A\u0301A\u0302A\u0303A\u0308A\u030a"
      u8"a\u0300a\u0301a\u0302a\u0303a\u0308a\u030a"
      u8"E\u0300E\u0301E\u0302E\u0308e\u0300e\u0301e\u0302e\u0308"
      u8"I\u0300I\u0301I\u0302I\u0308i\u0300i\u0301i\u0302i\u0308"
      u8"O\u0300O\u0301O\u0302O\u0303O\u0308O\u0337"
      u8"o\u0300o\u0301o\u0302o\u0303o\u0308o\u0337"
      u8"U\u0300U\u0301U\u0302U\u0308u\u0300u\u0301u\u0302u\u0308"
      u8"C\u0327c\u0327N\u0303n\u0303"); // Y\u0301y\u0301 ");
  std::string normalized = text::normalize_latin_to_utf8(input);
  for (size_t i = 0; i < normalized.size(); i++) {
    if (normalized[i] != expected[i]) {
      std::cout << "Mismatch at " << i << ": expected '" << expected[i]
                << "' (0x" << std::hex << static_cast<int>(expected[i])
                << "), got '" << normalized[i] << "' (0x" << std::hex
                << static_cast<int>(normalized[i]) << ")\n";
    }
    EXPECT_EQ(normalized[i], expected[i]);
  }
  EXPECT_TRUE(normalized == expected);
}

TEST(Text, lowercasing) {
  std::string input = "Hello, World!";
  std::string expected = "hello, world!";
  std::string result = text::lowercase(input);
  EXPECT_EQ(result, expected);
  EXPECT_EQ(text::lowercase(result), result);
  std::filesystem::path p("C:\\Path\\To\\File.TXT");
  std::filesystem::path expected_path("C:\\Path\\To\\File.txt");
  files::lowercase_extension(p);
  EXPECT_EQ(p, expected_path);
}

TEST(Text, Base64_silliness) {
  std::uint32_t input = 2345678;
  std::uint64_t result = hacks::base64_string_as_int(input);
  std::string expected = "I8rO";
  std::string result_str(reinterpret_cast<const char*>(&result));
  EXPECT_EQ(result_str, expected);
  input = 0;
  result = hacks::base64_string_as_int(input);
  expected = "A";
  result_str = std::string(reinterpret_cast<const char*>(&result));
  EXPECT_EQ(result_str, expected);
  input = 1;
  result = hacks::base64_string_as_int(input);
  expected = "B";
  result_str = std::string(reinterpret_cast<const char*>(&result));
  EXPECT_EQ(result_str, expected);
}

TEST(Files, RootIterator) {
  bool found_slash = false;
  std::set<std::filesystem::path> roots;
  size_t count = 0;
  int len = -16384;
  for (const auto& root : files::drive_range{}) {
    found_slash |= (root == "/");
    roots.insert(root);
    count++;
    if (len == -16384) {
      len = root.string().size();
    } else if (root.string().size() != static_cast<size_t>(len)) {
      len = -1;
    }
    std::cout << "Found root: " << root << std::endl;
  }
#if defined(__linux__)
  EXPECT_TRUE(found_slash);
  EXPECT_TRUE(len == -1 ||
              count == 1); // Linux roots can be long (e.g. /media/usb)
#elif defined(__APPLE__)
  EXPECT_TRUE(found_slash);
  EXPECT_TRUE(len == -1 ||
              count == 1); // Mac roots can be long (e.g. /Volumes/Thumb)
#elif defined(_WIN32)
  EXPECT_FALSE(found_slash);
  EXPECT_TRUE(len == 3); // Windows roots should all be chars (e.g. C:\)
#else
  std::cout << "Testing on unknown OS\n";
#endif
  EXPECT_FALSE(roots.empty());
  // No duplicates:
  EXPECT_EQ(count, roots.size());
}
