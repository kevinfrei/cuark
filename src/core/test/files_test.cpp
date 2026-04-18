#include <filesystem>
#include <iostream>
#include <string>

#include <crow.h>
#include <gtest/gtest.h>

#include "files.hpp"

#if defined(_WIN32)
static const char* argv0 = "core_testing.exe";
#else
static const char* argv0 = "core_testing";
#endif

TEST(Files, Miscellany) {
  files::set_program_location(argv0);
  auto web_dir = files::get_web_dir();
  std::filesystem::path index = web_dir / "index.html";
  EXPECT_TRUE(std::filesystem::exists(index));
  EXPECT_STREQ(files::get_app_name().string().c_str(), "core_testing");
  auto contents = files::read_file(index);
  EXPECT_TRUE(contents);
  EXPECT_FALSE(contents->find("window.wsport = 42;") == contents->npos);
}

TEST(Files, Name_encoding) {
  const std::string bad_chars = "\n\t\r\\/:?*\"<>|. ";
  const std::string bonkers = "c:\\AbC?*t";
  auto encoded = files::file_name_encode(bonkers);
  std::cout << encoded << std::endl;
  EXPECT_TRUE(bonkers.length() < encoded.string().length());
  auto decoded = files::file_name_decode(encoded.string());
  EXPECT_STREQ(bonkers.c_str(), decoded->c_str());
  for (const char c : encoded.string()) {
    EXPECT_TRUE(bad_chars.find(c) == bad_chars.npos);
  }
  encoded = files::file_name_encode(bad_chars);
  EXPECT_TRUE(bad_chars.length() < encoded.string().length());
  decoded = files::file_name_decode(encoded.string());
  EXPECT_STREQ(bad_chars.c_str(), decoded->c_str());
  for (const auto& c : encoded.string()) {
    EXPECT_TRUE(bad_chars.find(c) == bad_chars.npos);
  }
}

TEST(Files, MIME_detection) {
  EXPECT_STREQ(files::path_to_mime_type("file.txt").c_str(), "text/plain");
  EXPECT_STREQ(files::path_to_mime_type("styles.css").c_str(), "text/css");
  EXPECT_STREQ(files::path_to_mime_type("index.html").c_str(), "text/html");
  EXPECT_STREQ(files::path_to_mime_type("foo.htm").c_str(), "text/html");
  EXPECT_STREQ(files::path_to_mime_type("script.js").c_str(),
               "text/javascript");
  EXPECT_STREQ(files::path_to_mime_type("picture.gif").c_str(), "image/gif");
  EXPECT_STREQ(files::path_to_mime_type("image.jpg").c_str(), "image/jpeg");
  EXPECT_STREQ(files::path_to_mime_type("photo.png").c_str(), "image/png");
  EXPECT_STREQ(files::path_to_mime_type("photo.svg").c_str(), "image/svg+xml");
  EXPECT_STREQ(files::path_to_mime_type("tunes.flac").c_str(), "audio/flac");
  EXPECT_STREQ(files::path_to_mime_type("music.m4a").c_str(), "audio/m4a");
  EXPECT_STREQ(files::path_to_mime_type("music.mp3").c_str(), "audio/mp3");
  EXPECT_STREQ(files::path_to_mime_type("foo.js.map").c_str(),
               "application/json");
  EXPECT_STREQ(files::path_to_mime_type("data.json").c_str(),
               "application/json");
  EXPECT_STREQ(files::path_to_mime_type("data.ptr").c_str(), "text/html");
}
