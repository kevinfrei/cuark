export module core.quitting;

import std;

namespace quitting {

std::atomic_bool quit = false;
std::atomic_int8_t quit_timer = 10;

#if defined(_DEBUG)
bool debug_quit = false;
#else
bool debug_quit = true;
#endif

export void keep_alive() {
  quit_timer.store(10);
}

export bool should_quit() {
  return quit && debug_quit;
}

export void really_quit() {
  quit = true;
  debug_quit = true;
}

export void loop_wait() {
  if (quit_timer > 0) {
    quit_timer--;
  } else {
    quit = true;
  }
}

} // namespace quitting