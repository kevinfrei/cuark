module;

#include <cstdint>
#include <utility>

export module tools.hacks;

export {

  namespace hacks {

  // Base 64: A-Za-z0-9+/
  // My silly Base 64: A-Za-z0-9-_
  // This is a very simple base64 encoding that uses '-' and '_' instead of '+'
  // and '/' It is not a standard base64 encoding, but I use it so it's URL
  // safe.
  std::uint64_t base64_string_as_int(std::uint32_t val) {
    char buffer[sizeof(std::uint64_t)];
    int i = 0;
    for (; i < 6 && (val || (i == 0)); i++) {
      buffer[i] =
          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
              [val & 0x3f];
      val >>= 6;
    }
    // Reverse the buffer:
    for (int j = 0; j < i / 2; j++) {
      std::swap(buffer[j], buffer[i - j - 1]);
    }
    // Null-terminate the string:
    buffer[i] = '\0';
    std::uint64_t result = *reinterpret_cast<std::uint64_t*>(&buffer[0]);
    return result;
  }

  } // namespace hacks
}