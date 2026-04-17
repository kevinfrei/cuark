#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <crow.h>
#include <gtest/gtest.h>

#include "CommonTypes.hpp"
#include "json_pickling.hpp"

TEST(CommonTypes, Keys) {
  crow::json::wvalue json_value = to_json(Shared::Keys::Settings);
  EXPECT_EQ(json_value.t(), crow::json::type::String);
  std::string s = json_value.dump();
  crow::json::rvalue json_value2 = crow::json::load(s);
  std::optional<Shared::Keys> key_value = from_json<Shared::Keys>(json_value2);
  EXPECT_TRUE(key_value.has_value());
  EXPECT_EQ(key_value.value(), Shared::Keys::Settings);
  auto bool_value = from_json<bool>(json_value2);
  EXPECT_FALSE(bool_value.has_value());
}

TEST(CommonTypes, StrId) {
  crow::json::wvalue json_value = to_json(Shared::StrId::FilePath);
  EXPECT_EQ(json_value.t(), crow::json::type::String);
  std::string s = json_value.dump();
  crow::json::rvalue json_value2 = crow::json::load(s);
  std::optional<Shared::StrId> strid_value =
      from_json<Shared::StrId>(json_value2);
  EXPECT_TRUE(strid_value.has_value());
  EXPECT_EQ(strid_value.value(), Shared::StrId::FilePath);
  auto key_value = from_json<Shared::Keys>(json_value2);
  EXPECT_FALSE(key_value.has_value());
}

TEST(CommonTypes, CurrentView) {
  crow::json::wvalue json_value = to_json(Shared::CurrentView::settings);
  EXPECT_EQ(json_value.t(), crow::json::type::Number);
  std::string s = json_value.dump();
  crow::json::rvalue json_value2 = crow::json::load(s);
  std::optional<Shared::CurrentView> curview_value =
      from_json<Shared::CurrentView>(json_value2);
  EXPECT_TRUE(curview_value.has_value());
  EXPECT_EQ(curview_value.value(), Shared::CurrentView::settings);
  auto str_val = from_json<std::string>(json_value2);
  EXPECT_FALSE(str_val.has_value());
}

TEST(CommonTypes, IpcCall) {
  crow::json::wvalue json_value = to_json(Shared::IpcCall::ReadFromStorage);
  EXPECT_EQ(json_value.t(), crow::json::type::Number);
  std::string s = json_value.dump();
  crow::json::rvalue json_value2 = crow::json::load(s);
  std::optional<Shared::IpcCall> ipccall_value =
      from_json<Shared::IpcCall>(json_value2);
  EXPECT_TRUE(ipccall_value.has_value());
  EXPECT_EQ(ipccall_value.value(), Shared::IpcCall::ReadFromStorage);
  auto str_val = from_json<Shared::Keys>(json_value2);
  EXPECT_FALSE(str_val.has_value());
}

TEST(CommonTypes, SocketMsg) {
  crow::json::wvalue json_value = to_json(Shared::SocketMsg::ContentLoaded);
  EXPECT_EQ(json_value.t(), crow::json::type::Number);
  std::string s = json_value.dump();
  crow::json::rvalue json_value2 = crow::json::load(s);
  std::optional<Shared::SocketMsg> socketmsg_value =
      from_json<Shared::SocketMsg>(json_value2);
  EXPECT_TRUE(socketmsg_value.has_value());
  EXPECT_EQ(socketmsg_value.value(), Shared::SocketMsg::ContentLoaded);
  auto str_val = from_json<Shared::StrId>(json_value2);
  EXPECT_FALSE(str_val.has_value());
}

TEST(CommonTypes, StorageId) {
  crow::json::wvalue json_value = to_json(Shared::StorageId::CurrentView);
  EXPECT_EQ(json_value.t(), crow::json::type::String);
  std::string s = json_value.dump();
  crow::json::rvalue json_value2 = crow::json::load(s);
  std::optional<Shared::StorageId> storageid_value =
      from_json<Shared::StorageId>(json_value2);
  EXPECT_TRUE(storageid_value.has_value());
  EXPECT_EQ(storageid_value.value(), Shared::StorageId::CurrentView);
  auto str_val = from_json<Shared::Keys>(json_value2);
  EXPECT_FALSE(str_val.has_value());
}

TEST(CommonTypes, MimeData) {
  Shared::MimeData mime_data;
  mime_data.type = "text/plain";
  mime_data.data = "Hello, world!";
  crow::json::wvalue json_value = to_json(mime_data);
  EXPECT_EQ(json_value.t(), crow::json::type::Object);
  std::string s = json_value.dump();
  crow::json::rvalue json_value2 = crow::json::load(s);
  std::optional<Shared::MimeData> mime_data_value =
      from_json<Shared::MimeData>(json_value2);
  EXPECT_TRUE(mime_data_value.has_value());
  EXPECT_EQ(mime_data_value->type, "text/plain");
  EXPECT_EQ(mime_data_value->data, "Hello, world!");
}

TEST(CommonTypes, FileFilterItem) {
  Shared::FileFilterItem ffi;
  ffi.name = "Audio Files";
  ffi.extensions = {"mp3", "wav", "flac"};
  crow::json::wvalue json_value = to_json(ffi);
  EXPECT_EQ(json_value.t(), crow::json::type::Object);
  std::string s = json_value.dump();
  crow::json::rvalue json_value2 = crow::json::load(s);
  std::optional<Shared::FileFilterItem> ffi_value =
      from_json<Shared::FileFilterItem>(json_value2);
  EXPECT_TRUE(ffi_value.has_value());
  EXPECT_EQ(ffi_value->name, "Audio Files");
  EXPECT_EQ(ffi_value->extensions.size(), 3);
  auto b = ffi_value->extensions.cbegin();
  auto e = ffi_value->extensions.cend();
  EXPECT_NE(std::find(b, e, "mp3"), e);
  EXPECT_NE(std::find(b, e, "wav"), e);
  EXPECT_NE(std::find(b, e, "flac"), e);
}

TEST(CommonTypes, OpenDialogOptions) {
  Shared::OpenDialogOptions odo;
  odo.folder = true;
  odo.title = "Select a folder";
  odo.defaultPath = "/home/user";
  odo.buttonLabel = "Choose";
  crow::json::wvalue json_value = to_json(odo);
  EXPECT_EQ(json_value.t(), crow::json::type::Object);
  std::string s = json_value.dump();
  crow::json::rvalue json_value2 = crow::json::load(s);
  auto odo_value = from_json<Shared::OpenDialogOptions>(json_value2);
  EXPECT_TRUE(odo_value.has_value());
  EXPECT_EQ(odo_value->folder, true);
  EXPECT_EQ(odo_value->title, "Select a folder");
  EXPECT_EQ(odo_value->defaultPath, "/home/user");
  EXPECT_EQ(odo_value->buttonLabel, "Choose");
  auto num_value = from_json<uint64_t>(json_value2);
  EXPECT_FALSE(num_value.has_value());
}
