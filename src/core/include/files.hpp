#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include <crow/http_response.h>

#include "CommonTypes.hpp"
namespace files {

void set_program_location(const char* argv0);
std::filesystem::path get_web_dir();
std::filesystem::path get_app_name();
std::string path_to_mime_type(const std::filesystem::path& file_path);
std::optional<std::string> file_name_decode(std::string_view filename);
std::filesystem::path file_name_encode(std::string_view filename);
std::optional<std::string> read_file(std::filesystem::path file_path);
std::string new_folder_picker(
    const std::optional<Shared::OpenDialogOptions>& odo);
void folder_picker(crow::response& resp, std::string_view data);

} // namespace files
