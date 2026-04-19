#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/websocket.h>

#include "range_tools.hpp"
#include "text_tools.hpp"
#include "tools.hpp"

#include "CommonTypes.hpp"

namespace handlers {

using RouteHandler = std::function<void(crow::response&, std::string_view)>;

// A static registry to hold your routes
extern std::unordered_map<Shared::IpcCall, RouteHandler> RouteTable;

namespace _internal_glue {

// This is the "glue" that hides the template types from the table

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
  // First, URL-decode each segment into the segments array. If there are too
  // many or too few, or if decoding fails, return a 404.
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
      if constexpr (std::is_void_v<ReturnType>) {
        // Void function: Just execute and return 204 (No Content) or 200
        handle_call(
            text::from_string<ArgT<ArgIndex, Args...>>(segments[ArgIndex])...);
      } else {
        auto result = handle_call(
            text::from_string<ArgT<ArgIndex, Args...>>(segments[ArgIndex])...);
        if constexpr (std::is_integral_v<ReturnType>) {
          resp.set_header("Content-Type", "application/text");
          resp.body = std::to_string(result);
        } else if constexpr (std::is_same_v<ReturnType, std::string> ||
                             std::is_same_v<ReturnType, std::string_view> ||
                             std::is_same_v<ReturnType, char*>) {
          resp.set_header("Content-Type", "application/text");
          resp.body = std::string(result);
        } else {
          // Assuming Crow's wvalue or similar JSON lib
          resp.set_header("Content-Type", "application/json");
          resp.body = to_json(result).dump();
        }
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

} // namespace _internal_glue

// The helper to "erase" the types
template <typename Func>
void register_route(const Shared::IpcCall& call, Func&& handler) {
  RouteTable[call] = [handler = std::forward<Func>(handler)](
                         crow::response& resp, std::string_view path) {
    _internal_glue::ValidateAndCall(resp, path, handler);
  };
}

void initialize_default_apis();
crow::response www_path(const crow::request& req, const std::string& path);
crow::response images(const crow::request&, const std::string& path);
crow::response tune(const crow::request& req, const std::string& path);
crow::response api(const crow::request& req, const std::string& path);
crow::response keepalive();
crow::response quit();
void socket_message(crow::websocket::connection& conn,
                    const std::string& data,
                    bool is_binary);

} // namespace handlers
