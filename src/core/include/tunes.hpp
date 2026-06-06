#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include <crow.h>

namespace tunes {

std::optional<std::filesystem::path> get_tune(const std::string& path);

} // namespace tunes
