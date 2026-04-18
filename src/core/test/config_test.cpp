#include <filesystem>
#include <optional>
#include <string>

#include <crow.h>
#include <gtest/gtest.h>

#include "files.hpp"

#include "config.hpp"

#if defined(_WIN32)
static const char* argv0 = "core_testing.exe";
#else
static const char* argv0 = "core_testing";
#endif

TEST(Config, TheBasics) {
  files::set_program_location(argv0);
  std::filesystem::path res = config::get_path();
  EXPECT_FALSE(res.empty());
  EXPECT_EQ(res.stem(), files::get_app_name());
}

TEST(Config, Storage) {
  files::set_program_location(argv0);
  // Test if the item is writable
  EXPECT_TRUE(config::write_to_storage("test_key", "test-value"));
  EXPECT_TRUE(config::write_to_storage("Test key", "test-value2"));
  auto read_value = config::read_from_storage("test_key");
  EXPECT_TRUE(read_value.has_value());
  EXPECT_EQ(read_value.value(), "test-value");
  // Test if the item can be overwritten
  EXPECT_TRUE(config::write_to_storage("test_key", "new-value"));
  read_value = config::read_from_storage("test_key");
  // Let's flush the cache to ensure we read from *storage*
  config::flush_storage_cache();
  EXPECT_TRUE(read_value.has_value());
  EXPECT_EQ(read_value.value(), "new-value");
  // Test if the item can be deleted
  EXPECT_TRUE(config::delete_from_storage("test_key"));
  read_value = config::read_from_storage("test_key");
  EXPECT_FALSE(read_value.has_value());
  // Test that the item is not found for deletion
  EXPECT_FALSE(config::delete_from_storage("test_key"));
}

TEST(Config, StorageNotifcations) {
  files::set_program_location(argv0);
  // Test if the item is writable
  std::int32_t id1, id2, id3;
  std::optional<std::string> old1, new1, old2, new2;
  config::listening_function listener1 =
      [&](std::optional<std::string_view> old_value,
          std::optional<std::string_view> new_value) {
        old1 = old_value;
        new1 = new_value;
      };
  config::listening_function listener2 =
      [&](std::optional<std::string_view> old_value,
          std::optional<std::string_view> new_value) {
        old2 = old_value;
        new2 = new_value;
      };
  config::clear_storage();
  id1 = config::subscribe_to_change("lk1", listener1);
  id2 = config::subscribe_to_change("lk2", listener2);
  id3 = config::subscribe_to_change("lk1", listener2);

  // Initial state:
  EXPECT_FALSE(old1.has_value());
  EXPECT_FALSE(new1.has_value());
  EXPECT_FALSE(old2.has_value());
  EXPECT_FALSE(new2.has_value());

  EXPECT_TRUE(config::write_to_storage("lk1", "test-value"));

  // Both listeners should be notified
  EXPECT_FALSE(old1.has_value());
  EXPECT_TRUE(new1.has_value());
  EXPECT_FALSE(old2.has_value());
  EXPECT_TRUE(new2.has_value());
  EXPECT_EQ(new1.value(), "test-value");
  EXPECT_EQ(new2.value(), "test-value");

  EXPECT_TRUE(config::write_to_storage("lk2", "test-value2"));
  // Only the second listener should have been called
  EXPECT_FALSE(old1.has_value());
  EXPECT_TRUE(new1.has_value());
  EXPECT_FALSE(old2.has_value());
  EXPECT_TRUE(new2.has_value());
  EXPECT_EQ(new1.value(), "test-value");
  EXPECT_EQ(new2.value(), "test-value2");

  EXPECT_TRUE(config::write_to_storage("lk1", "new-value"));
  // Both listeners should have been called again
  EXPECT_TRUE(old1.has_value());
  EXPECT_TRUE(new1.has_value());
  EXPECT_TRUE(old2.has_value());
  EXPECT_TRUE(new2.has_value());
  EXPECT_EQ(old1.value(), "test-value");
  EXPECT_EQ(new1.value(), "new-value");
  EXPECT_EQ(old2.value(), "test-value");
  EXPECT_EQ(new2.value(), "new-value");

  EXPECT_TRUE(config::write_to_storage("lk2", "new-value2"));
  // Only the second listener should have been called
  EXPECT_TRUE(old1.has_value());
  EXPECT_TRUE(new1.has_value());
  EXPECT_TRUE(old2.has_value());
  EXPECT_TRUE(new2.has_value());
  EXPECT_EQ(old1.value(), "test-value");
  EXPECT_EQ(new1.value(), "new-value");
  EXPECT_EQ(old2.value(), "test-value2");
  EXPECT_EQ(new2.value(), "new-value2");

  // For fun...
  config::flush_storage_cache();

  // Now let's unsubscribe the second listener from the first key
  EXPECT_TRUE(config::unsubscribe_from_change(id3));

  EXPECT_TRUE(config::delete_from_storage("lk1"));
  // Only the first listener should have been called
  EXPECT_TRUE(old1.has_value());
  EXPECT_FALSE(new1.has_value());
  EXPECT_TRUE(old2.has_value());
  EXPECT_TRUE(new2.has_value());
  EXPECT_EQ(old1.value(), "new-value");
  EXPECT_EQ(old2.value(), "test-value2");
  EXPECT_EQ(new2.value(), "new-value2");

  EXPECT_TRUE(config::delete_from_storage("lk2"));
  // Only the second listener should have been called
  EXPECT_TRUE(old1.has_value());
  EXPECT_FALSE(new1.has_value());
  EXPECT_TRUE(old2.has_value());
  EXPECT_FALSE(new2.has_value());
  EXPECT_EQ(old1.value(), "new-value");
  EXPECT_EQ(old2.value(), "new-value2");
  EXPECT_TRUE(config::unsubscribe_from_change(id1));
  EXPECT_TRUE(config::unsubscribe_from_change(id2));
  EXPECT_FALSE(config::unsubscribe_from_change(id1)); // Already unsubscribed
  EXPECT_FALSE(config::unsubscribe_from_change(id2)); // Already unsubscribed
  config::clear_storage();
}

/*
TEST(File, FilePicker) {
  auto resp = crow::response();
  files::folder_picker(resp, "Test data");
}
*/
