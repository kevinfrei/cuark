#pragma once

#include <cstdint>

namespace setup {

std::uint16_t get_random_port();
void init(int argc, const char* argv[]);
void run();

} // namespace setup
