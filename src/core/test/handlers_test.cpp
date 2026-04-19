#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include <crow.h>
#include <gtest/gtest.h>

#include "CommonTypes.hpp"

#include "handlers.hpp"

TEST(Handlers, SimpleStuff) {
  int num = 0;
  std::string_view strv = "nope";
  std::optional<int> flg = std::nullopt;
  handlers::register_route(
      Shared::IpcCall::MenuAction,
      [&](int number, std::string_view str, int flag) -> bool {
        strv = str;
        num = number;
        flg = flag;
        return true;
      });
  handlers::register_route(
      Shared::IpcCall::AsyncData,
      [&](int number, std::string_view str, int flag) -> bool {
        strv = str;
        num ^= number;
        flg = flag;
        return true;
      });
  handlers::api(crow::request{}, "10/42/hello/1");
  EXPECT_EQ(num, 42);
  EXPECT_EQ(strv, "hello");
  EXPECT_TRUE(flg.has_value());
  EXPECT_EQ(flg.value(), 1);
  handlers::api(crow::request{}, "9/14/world/1313");
  EXPECT_EQ(num, 42 ^ 14);
  EXPECT_EQ(strv, "world");
  EXPECT_TRUE(flg.has_value());
  EXPECT_EQ(flg.value(), 1313);
}

TEST(Handlers, ComplexArgs) {
  // I want to test things besides 32 bit integers when URL encoded:
  bool wasRun = false;
  handlers::register_route(Shared::IpcCall::WriteToStorage,
                           [&](int num, Shared::MimeData md) {
                             EXPECT_EQ(num, 123);
                             EXPECT_EQ(md.type, "text");
                             EXPECT_EQ(md.data, "bar");
                             wasRun = true;
                           });
  handlers::api(crow::request{},
                "2/123/%7B%22type%22%3A%22text%22%2C%22data%22%3A%22bar%22%7D");
  EXPECT_TRUE(wasRun);
}
