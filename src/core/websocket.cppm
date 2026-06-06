module;

#include <crow.h>

export module core.websocket;

import std;

import core.handler;
import ts_cpp_idl.Shared;

namespace websocket {

crow::websocket::connection* webSocket = nullptr;

export void configure(crow::SimpleApp& app) {
  CROW_WEBSOCKET_ROUTE(app, "/ws")
      .onopen([&](crow::websocket::connection& conn) -> void {
        CROW_LOG_INFO << "WebSocket connection opened from "
                      << conn.get_remote_ip();
        webSocket = &conn;
      })
      .onmessage(handlers::socket_message)
      .onclose([&](crow::websocket::connection& conn,
                   const std::string& reason,
                   uint16_t /*code*/) {
        CROW_LOG_INFO
            << "WebSocket connection closed from " << conn.get_remote_ip()
            << " with reason: " << reason /* << " and code " << code */;
        webSocket = nullptr;
      });
}

export void keep_alive() {
  if (webSocket) {
    webSocket->send_text(
        std::to_string(static_cast<uint32_t>(Shared::SocketMsg::KeepAlive)) +
        ";");
  }
}

} // namespace websocket