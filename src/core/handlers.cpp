#include <array>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <tuple>

#include <crow.h>
#include <crow/logging.h>

#include "CommonTypes.hpp"
#include "api.hpp"
#include "config.hpp"
#include "files.hpp"
#include "images.hpp"
#include "json_pickling.hpp"
#include "quitting.hpp"
#include "range_tools.hpp"
#include "setup.hpp"
#include "text_tools.hpp"
#include "tools.hpp"
#include "tunes.hpp"

#include "handlers.hpp"

namespace handlers {

crow::response www_path(const crow::request&, const std::string& path) {
  quitting::keep_alive();

  CROW_LOG_INFO << "Path: " << path;
  crow::response resp;
  std::filesystem::path p =
      files::get_web_dir() / (path.empty() ? "index.html" : path);
  if (p.filename() == "index.html") {
    // If we're sending index.html, we should *clear* ready
    config::not_ready();

    // We need to process the index.html file to replace the websocket URL
    // with the correct one.

    // Read the contents of the index.html file,
    // Replace "window.wsport = 42;" with the actual port number
    std::ifstream file(p);
    if (!file.is_open()) {
      tools::e404(resp, "index.html not found");
      return resp;
    }
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    // Replace the placeholder with the actual port number
    std::string wsport = std::to_string(setup::get_random_port());
    size_t pos = content.find("window.wsport = 42;");
    if (pos != std::string::npos) {
      content.replace(pos, 20, "window.wsport = " + wsport + ";");
      resp.body = content;
      resp.code = 200;
      resp.set_header("Content-Type", "text/html");
      CROW_LOG_INFO << "Sending transformed index.html";
    } else {
      CROW_LOG_INFO << "Placeholder not found in index.html file: "
                    << p.generic_string();
      resp.code = 500;
      resp.body = "Internal Server Error";
      resp.set_header("Content-Type", "text/plain");
    }
  } else {
    CROW_LOG_INFO << "Sending raw file " << p.generic_string();
    resp.set_static_file_info_unsafe(p.generic_string());
    resp.set_header("Content-type", files::path_to_mime_type(p));
  }
  return resp;
}

crow::response images(const crow::request&, const std::string& query) {
  quitting::keep_alive();
  crow::response resp;
  std::filesystem::path p = image::get_image_path(query);
  CROW_LOG_INFO << "Images " << p.generic_string();
  resp.set_static_file_info_unsafe(p.generic_string());
  resp.set_header("Content-type", files::path_to_mime_type(p));
  return resp;
}

crow::response keepalive() {
  quitting::keep_alive();
  crow::response resp;
  resp.code = 200;
  resp.body = "OK";
  resp.set_header("Content-Type", "text/plain");
  return resp;
}

crow::response quit() {
  quitting::really_quit();
  return crow::response(200);
}

// TODO: Make this actually validate the Range header
struct range_header {
  std::uint64_t start;
  std::uint64_t end;
  bool start_present;
  bool end_present;
};

std::optional<range_header> validate_range_header(const std::string& range) {
  std::optional<range_header> res;
  if (range.substr(0, 6) != "bytes=") {
    return res;
  }
  size_t dashPos = range.find('-');
  if (dashPos == std::string::npos) {
    return res;
  }
  range_header rh;
  rh.start_present = false;
  rh.end_present = false;
  if (dashPos > 6) {
    rh.start = std::stoull(range.substr(6, dashPos - 6));
    rh.start_present = true;
  }
  if (dashPos + 1 < range.size()) {
    rh.end = std::stoull(range.substr(dashPos + 1));
    rh.end_present = true;
  }
  if (rh.start_present || rh.end_present) {
    res = rh;
  }
  return res;
}

crow::response tune(const crow::request& req, const std::string& path) {
  quitting::keep_alive();
  crow::response resp;
  auto maybe_song = tunes::get_tune(path);
  if (!maybe_song) {
    tools::e404(resp, "Tune not found");
    return resp;
  }
  /*
  A minimal implementation needs to:
  Parse the Range header
  Compute the correct byte offsets
  Return 206 Partial Content
  Include Content-Range and Accept-Ranges: bytes
  Send only the requested slice of the file
  */
  const auto& range = req.headers.find("Range");
  range_header rh;
  if (range == req.headers.end()) {
    resp.code = 416;
    return resp;
  }
  CROW_LOG_INFO << "Range header: " << range->second;
  auto maybe_range = validate_range_header(range->second);
  if (!maybe_range.has_value()) {
    // TODO: Handle weirder ranges?
    resp.code = 416;
    return resp;
  } else {
    rh = maybe_range.value();
  }
  const auto& song = maybe_song.value();
  // TODO: Get the file size, check to see we can send the amount requested.
  // If we can, go ahead & send it.
  // Common case: Safari asks for 0-1 for audio files, presumably to detect
  // the total file size?
  if (rh.start_present && rh.end_present && rh.start == 0 && rh.end == 1) {
    // Send the two starting bytes for 'song':
    std::ifstream file(song, std::ios::binary);
    if (file.is_open()) {
      char buffer[2];
      file.read(buffer, 2);
      resp.body = std::string(buffer, 2);
      resp.code = 206;
      resp.set_header("Content-Type", files::path_to_mime_type(song));
      resp.set_header("Accept-Ranges", "bytes");
      resp.set_header(
          "Content-Range",
          "bytes 0-1/" + std::to_string(std::filesystem::file_size(song)));
      return resp;
    }
  }
  resp.set_static_file_info_unsafe(song.generic_string());
  resp.set_header("Content-type", files::path_to_mime_type(song));
  resp.set_header("Accept-Ranges", "bytes");
  std::size_t fileSize = std::filesystem::file_size(song);
  std::ostringstream o;
  o << "bytes 0-" << fileSize - 1 << "/" << fileSize;
  CROW_LOG_DEBUG << o.str();
  resp.set_header("Content-Range", o.str());
  resp.code = 206; // Partial Content
  return resp;
}

#pragma region Template Magic for API Calls

// Helper to index a parameter-pack of argument types. Takes the index and
// the parameter-pack so it can be used at namespace scope.
template <size_t ArgIndex, typename... Args>
using ArgT = std::tuple_element_t<ArgIndex, std::tuple<std::decay_t<Args>...>>;

// Internal worker that deduces arg types
template <typename... Args, typename Func>
void Execute(crow::response& resp, std::string_view path, Func&& handle_call) {
  constexpr size_t ArgCount = sizeof...(Args);
  std::array<std::string, ArgCount> segments;
  auto pos = segments.begin();
  auto path_chunks = path | views::split_string_view('/');
  size_t count = 0;
  for (auto const segment : path_chunks) {
    if (pos == segments.end()) {
      tools::e404(resp, "Too many parameters: " + std::string(path));
      return;
    }
    if (auto decoded = tools::url_decode(segment)) {
      *pos++ = std::move(*decoded);
    } else {
      tools::e404(resp, "URL Decode Failed for " + std::string(segment));
      return;
    }
    count++;
  }
  if (count != ArgCount) {
    tools::e404(resp, "Missing parameters from " + std::string(path));
    return;
  }

  // Unpack and Call
  auto call_and_respond = [&]<size_t... ArgIndex>(
      std::index_sequence<ArgIndex...>) {
    using ReturnType = std::invoke_result_t<Func, std::decay_t<Args>...>;

    try {
      // Returning function: Execute and encode result
      // Use the Is... index pack to pick the corresponding type from the
      // Args... pack by indexing into a tuple of the decayed argument types.
      auto result = handle_call(
          text::from_string<ArgT<ArgIndex, Args...>>(segments[ArgIndex])...);
      if constexpr (std::is_void_v<ReturnType>) {
        // Void function: Just execute and return 204 (No Content) or 200
      } else if constexpr (std::is_integral_v<ReturnType>) {
        resp.set_header("Content-Type", "application/text");
        resp.body = std::to_string(result);
      } else if constexpr (std::is_same_v<ReturnType, std::string> ||
                           std::is_same_v<ReturnType, std::string_view> ||
                           std::is_same_v<ReturnType, char*>) {
        resp.set_header("Content-Type", "application/text");
        resp.body = std::string(result);
      } else {
        // Assuming Crow's wvalue or similar JSON lib
        // crow::json::wvalue json_out;
        // json_out["result"] = result;

        resp.set_header("Content-Type", "application/json");
        resp.body = to_json(result).dump();
      }
      resp.code = 200;
      resp.end();
    } catch (const std::exception& e) {
      // This should be a 500
      tools::e404(resp, std::string("Internal Error: ") + e.what());
    }
  };

  call_and_respond(std::make_index_sequence<ArgCount>{});
}

// Overload A: For regular functions: bool myFunc(string_view, int)
template <typename R, typename... Args>
void InternalCall(crow::response& resp,
                  std::string_view remaining,
                  R (*func)(Args...),
                  R (*)(Args...)) {
  Execute<Args...>(resp, remaining, func);
}

// Overload B: For lambdas/functors: has an operator()
template <typename Func, typename R, typename... Args>
void InternalCall(crow::response& resp,
                  std::string_view remaining,
                  Func&& handle_call,
                  R (std::remove_reference_t<Func>::*)(Args...) const) {
  Execute<Args...>(resp, remaining, std::forward<Func>(handle_call));
}

// "Public" API to use
template <typename Func>
void ValidateAndCall(crow::response& resp,
                     std::string_view path,
                     Func&& handle_call) {
  // Pass the pointer to the lambda's operator() to trigger deduction
  // Use constexpr if to decide which pointer to pass for deduction
  if constexpr (std::is_function_v<
                    std::remove_pointer_t<std::remove_reference_t<Func>>>) {
    InternalCall(resp, path, handle_call, handle_call);
  } else {
    InternalCall(resp,
                 path,
                 std::forward<Func>(handle_call),
                 &std::remove_reference_t<Func>::operator());
  }
}

#pragma endregion

// The URL comes in looking like this:
// https://.../api/<call-id>{/arg1/arg2/etc...} so the_path is
crow::response api(const crow::request&, const std::string& the_path) {
  quitting::keep_alive();
  std::string_view path(the_path);
  CROW_LOG_DEBUG << "API Path: " << path;
  crow::response resp;
  size_t slash = path.find('/');
  if (slash == std::string_view::npos) {
    slash = path.length();
  }
  Shared::IpcCall callId =
      text::from_string<Shared::IpcCall>(path.substr(0, slash));
  if (!Shared::is_valid(callId)) {
    tools::e404(resp, "Unknown API for path " + std::string(path));
    return resp;
  }
  // TODO: Finish stuff from here:
  path = path.substr(slash + 1, path.length() - (slash + 1));
  switch (callId) {
    case Shared::IpcCall::WriteToStorage:
      ValidateAndCall(resp, path, config::write_to_storage);
      break;
    case Shared::IpcCall::ReadFromStorage:
      ValidateAndCall(resp, path, config::read_from_storage);
      break;
    case Shared::IpcCall::DeleteFromStorage:
      ValidateAndCall(resp, path, config::delete_from_storage);
      break;
    case Shared::IpcCall::ShowOpenDialog:
      ValidateAndCall(resp, path, files::new_folder_picker);
      break;
    case Shared::IpcCall::AsyncData:
    case Shared::IpcCall::IsDev:
    case Shared::IpcCall::MenuAction:
    case Shared::IpcCall::MinimizeWindow:
    case Shared::IpcCall::MaximizeWindow:
    case Shared::IpcCall::RestoreWindow:
    case Shared::IpcCall::CloseWindow:
      CROW_LOG_ERROR
          << "Unimplemented API call received: " << Shared::to_string(callId)
          << " (" << underlying_cast(callId) << ") [" << path << "]";
      break;

    case Shared::IpcCall::Unknown:
    default:
      CROW_LOG_ERROR << "Unknown API call received: " << underlying_cast(callId)
                     << " [" << path << "]";
      std::string error_message =
          Shared::is_valid(callId)
              ? "Unimplemented API call: " +
                    std::string(Shared::to_string(callId))
              : "Unknown API call: " + std::to_string(underlying_cast(callId));
      tools::e404(resp, error_message);
      return resp;
  }

  return resp;
}

void socket_message(crow::websocket::connection& /* conn */,
                    const std::string& data,
                    bool /* is_binary */) {
  CROW_LOG_INFO << "Got a message from the client:" << data;
  // Message is IpcMessage;[json-formatted array of arguments]
  size_t pos = data.find(';');
  if (pos == data.npos) {
    CROW_LOG_ERROR << "Invalid websocket message received: " << data;
    return;
  }
  auto maybeMsg =
      text::to_integer<Shared::SocketMsg>(std::string_view{data.c_str(), pos});
  if (!maybeMsg) {
    CROW_LOG_ERROR << "Invalid websocket message received: " << data;
    return;
  }
  Shared::SocketMsg msg = *maybeMsg;
  if (!Shared::is_valid(msg)) {
    CROW_LOG_ERROR << "Invalid Socket message received: " << data;
    return;
  }
  // This is the only message we support *receiving* from the client
  switch (msg) {
    case Shared::SocketMsg::ContentLoaded:
      CROW_LOG_INFO << "Client finished loading content.\n"
                    << "marking config as ready and sending music db";
      config::set_ready();
      break;
    default: // Unsupported message
      CROW_LOG_ERROR
          << "Unsupported message received: " << Shared::to_string(msg) << " ("
          << underlying_cast(msg) << ") [" << data << "]";
  }
}

} // namespace handlers
