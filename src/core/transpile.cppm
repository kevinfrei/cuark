module;

#include <iostream>
#include <string>
#include <vector>

#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>

export module core.transpile;

namespace transpile {
export std::string bun_ts_to_js(const std::string& tsCode) {
  reproc::process bun;

  // Define the command (read from stdin)
  std::vector<std::string> args = {"bun",
                                   "-e",
                                   "console.write(new Bun.Transpiler({"
                                   "loader:'tsx',"
                                   "target:'browser'"
                                   "})"
                                   ".transformSync(await Bun.stdin.text()))"};
  // Start the process
  reproc::options options;
  options.redirect.in.type = reproc::redirect::pipe;
  options.redirect.out.type = reproc::redirect::pipe;

  std::error_code ec = bun.start(args, options);
  if (ec) {
    return "Error starting Bun: " + ec.message();
  }

  // Feed the TypeScript code to stdin
  // We must close stdin after writing so Bun knows we're done
  auto [bytes_written, ec_write] =
      bun.write(reinterpret_cast<const uint8_t*>(tsCode.data()), tsCode.size());
  std::cout << "Bun transpiler wrote " << bytes_written << " of "
            << tsCode.size() << " bytes" << std::endl;
  if (ec_write) {
    return "Error writing to Bun: " + ec_write.message();
  }

  ec = bun.close(reproc::stream::in);

  if (ec) {
    return "Error closing Bun stdin: " + ec.message();
  }

  // 4. Capture the JS output from stdout
  std::string jsOutput;
  reproc::sink::string sink(jsOutput);
  ec = reproc::drain(bun, sink, reproc::sink::null);

  if (ec) {
    return "Error reading from Bun: " + ec.message();
  }

  // 5. Wait for process to exit
  auto [status, ec_wait] = bun.wait(reproc::infinite);

  if (ec_wait) {
    return "Error waiting for Bun to exit: " + ec_wait.message();
  }
  std::cout << "Bun transpiler exited with status " << status << std::endl;

  return jsOutput;
}

} // namespace transpile