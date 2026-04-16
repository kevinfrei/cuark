#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string_view>

namespace config {

using listening_function = std::function<void(std::optional<std::string_view>,
                                              std::optional<std::string_view>)>;

std::filesystem::path get_path();
std::filesystem::path get_playlist_path();

bool write_to_storage(std::string_view key, std::string_view value);
std::optional<std::string> read_from_storage(std::string_view key);
bool delete_from_storage(std::string_view key);

std::int32_t subscribe_to_change(std::string_view key,
                                 listening_function callback);
bool unsubscribe_from_change(std::int32_t listener_id);

void flush_storage_cache();
void clear_storage();

void set_ready();
bool is_ready();
void not_ready();

} // namespace config
