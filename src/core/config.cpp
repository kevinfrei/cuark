#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include <sago/platform_folders.h>

#include "files.hpp"

#include "config.hpp"

// Cuz I'm lazy:
namespace fs = std::filesystem;

namespace config {

using read_lock = std::shared_lock<std::shared_mutex>;
using write_lock = std::unique_lock<std::shared_mutex>;

namespace {
// This is a singleton pattern to ensure we only initialize the paths once.
bool inited = false;
std::shared_mutex init_mutex;
fs::path cfg_path;
bool config_ready = false;

// This is the path to the configuration directory for the applicatio
void init() {
  read_lock lock(init_mutex);
  if (!inited) {
    lock.unlock(); // Release the lock before doing any I/O
    write_lock write_lock(init_mutex);
    if (inited) {
      return; // Another thread initialized it while we were waiting
    }
    cfg_path = sago::getStateDir();
    inited = true;
  }
}

} // namespace

void set_ready() {
  write_lock lock(init_mutex);
  config_ready = true;
}

void not_ready() {
  write_lock lock(init_mutex);
  config_ready = false;
}

bool is_ready() {
  read_lock lock(init_mutex);
  return config_ready;
}

// Returns the path to the configuration directory for the application.
fs::path get_path() {
  init();
  return fs::weakly_canonical(cfg_path / files::get_app_name());
}

fs::path get_persistence_path() {
  fs::path res = get_path() / "persistence";
  if (!fs::exists(res) && !fs::create_directories(res)) {
    CROW_LOG_ERROR << "Failed to create persistence directory: "
                   << res.string();
  }
  return res;
}

fs::path get_playlist_path() {
  fs::path res = get_persistence_path() / "playlists";
  if (!fs::exists(res) && !fs::create_directories(res)) {
    CROW_LOG_ERROR << "Failed to create playlists directory: " << res.string();
  }
  return res;
}

// Stuff to keep the cache & disk storage in sync & thread-safe.
std::unordered_map<std::string, std::string> cache;
std::shared_mutex the_mutex;
std::unordered_map<std::string, std::map<std::int32_t, listening_function>>
    listeners;
std::unordered_map<std::int32_t, std::string> listening_keys;

bool has_listener(std::string_view key) {
  std::string key_str{key};
  read_lock lock(the_mutex);
  auto it = listeners.find(key_str);
  return it != listeners.end() && !it->second.empty();
}

void notify_listeners(std::string_view key,
                      std::optional<std::string> maybe_value,
                      std::optional<std::string_view> new_value) {
  // Notify listeners about the change
  // Let's fill a vector with callbacks, so we don't call them while holding
  // the read lock.
  std::string key_str{key};
  std::vector<listening_function> callbacks;
  {
    read_lock lock(the_mutex);
    auto it = listeners.find(key_str);
    if (it != listeners.end()) {
      for (const auto& [listener_id, callback] : it->second) {
        callbacks.push_back(callback);
      }
    }
  }
  std::optional<std::string_view> old_value;
  if (maybe_value)
    old_value = *maybe_value;
  // Now call the callbacks outside of the lock
  for (const auto& callback : callbacks)
    callback(old_value, new_value);
}

bool write_to_storage(std::string_view key, std::string_view value) {
  std::optional<std::string> maybe_value;
  bool need_notification = has_listener(key);
  if (need_notification) {
    // If we have a listener for this key, we need to read the current value
    // from the storage first to avoid overwriting it with an empty value.
    maybe_value = read_from_storage(key);
    if (maybe_value && *maybe_value == value) {
      // If the value is the same as the current one, no need to write.
      return true;
    }
  }
  {
    auto path_to_data = get_persistence_path() / files::file_name_encode(key);
    write_lock lock(the_mutex);
    if (fs::exists(path_to_data)) {
      if (!fs::remove(path_to_data)) {
        CROW_LOG_ERROR << "Failed to remove existing file: "
                       << path_to_data.string();
        return false;
      }
    }
    std::ofstream out(path_to_data);
    if (!out) {
      CROW_LOG_ERROR << "Failed to open file for writing: "
                     << path_to_data.string();
      return false;
    }
    out << value;
    cache[std::string{key}] = value;
  }
  if (need_notification) {
    notify_listeners(key, maybe_value, value);
  }
  return true;
}

std::optional<std::string> read_from_storage(std::string_view key) {
  std::string key_str{key};
  // Read from the cache first
  {
    read_lock lock(the_mutex);
    auto it = cache.find(key_str);
    if (it != cache.end()) {
      return it->second;
    }
  }
  // We're going to have to modify the cache, so we'll need a write lock.
  // It's helpful to grab it before we do the disk read, so we don't have to
  // deal with races from the write_to_storage function.
  auto path_to_data = get_persistence_path() / files::file_name_encode(key);
  write_lock write_lock(the_mutex);
  if (!fs::exists(path_to_data)) {
    return std::nullopt; // Key does not exist
  }
  auto maybe_value = files::read_file(path_to_data);
  if (!maybe_value) {
    return std::nullopt; // Failed to read the file
  }
  // Release the read lock before modifying the cache
  cache[key_str] = *maybe_value; // Cache the value
  return *maybe_value;
}

bool delete_from_storage(std::string_view key) {
  std::optional<std::string> maybe_value;
  bool need_notification = has_listener(key);
  if (need_notification) {
    // If we have a listener for this key, we need to read the current value
    // from the storage first to call any listeners.
    maybe_value = read_from_storage(key);
    if (!maybe_value) {
      return false;
    }
  }

  {
    write_lock lock(the_mutex);
    cache.erase(std::string{key});
    auto path_to_data = get_persistence_path() / files::file_name_encode(key);
    if (!fs::exists(path_to_data)) {
      return false; // Key does not exist
    }
    if (!fs::remove(path_to_data)) {
      CROW_LOG_ERROR << "Failed to remove file: " << path_to_data.string();
      return false; // Failed to delete the file
    }
  }
  if (need_notification) {
    // Notify listeners about the deletion
    notify_listeners(key, maybe_value, std::nullopt);
  }
  return true;
}

void flush_storage_cache() {
  write_lock lock(the_mutex);
  cache.clear();
}

void clear_storage() {
  flush_storage_cache();
  auto path_to_data = get_persistence_path();
  if (fs::exists(path_to_data)) {
    write_lock lock(the_mutex);
    if (!fs::remove_all(path_to_data)) {
      CROW_LOG_ERROR << "Failed to clear storage directory: "
                     << path_to_data.string();
      return;
    }
  }
}

int32_t next_listener_id = 0;

std::int32_t subscribe_to_change(std::string_view key,
                                 listening_function callback) {
  std::string key_str{key};
  write_lock lock(the_mutex);
  if (listeners.find(key_str) == listeners.end()) {
    listeners[key_str] = {};
  }
  listeners[key_str][next_listener_id] = callback;
  listening_keys.insert({next_listener_id, key_str});
  CROW_LOG_INFO << "Subscribed to changes for key: " << key_str
                << " with listener ID: " << next_listener_id;
  return next_listener_id++;
}

bool unsubscribe_from_change(std::int32_t listener_id) {
  write_lock lock(the_mutex);
  auto lk = listening_keys.find(listener_id);
  if (lk == listening_keys.end()) {
    return false; // Listener ID not found
  }
  auto& key = lk->second;
  auto listeners_for_key = listeners.find(key);
  if (listeners_for_key == listeners.end()) {
    return false; // Key not found in listeners
  }
  auto listener = listeners_for_key->second.find(listener_id);
  if (listener == listeners_for_key->second.end()) {
    return false; // Listener ID not found for the key
  }
  CROW_LOG_INFO << "Unsubscribed from changes for key: " << key
                << " with listener ID: " << listener_id;
  listeners_for_key->second.erase(listener);
  listening_keys.erase(lk);
  return true;
}

} // namespace config
