module;

#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>

export module os.tools.file;

namespace os::tools::file {

export bool is_hidden_file_os(const std::filesystem::path& /*path*/) {
  return false;
}

void add_shortest(std::map<std::uint64_t, std::string>& roots,
                  std::string mount_point) {
  struct stat st;
  stat(mount_point.c_str(), &st);
  std::uint64_t val = st.st_dev;
  auto cur = roots.find(val);
  if (cur == roots.end()) {
    roots.insert(std::make_pair(val, mount_point));
    return;
  }
  if (cur->second.size() < mount_point.size()) {
    return;
  }
  cur->second = mount_point;
}

export std::vector<std::filesystem::path> populate_roots_os(bool) {
  // Always include the root
  std::map<std::uint64_t, std::string> roots;
  add_shortest(roots, "/");

  // Linux: Parse /proc/mounts for real block devices
  std::ifstream mounts("/proc/mounts");
  std::string line;
  while (std::getline(mounts, line)) {
    std::istringstream iss(line);
    std::string device, mountpoint, type;
    if (!(iss >> device >> mountpoint >> type))
      continue;

    // Filter for "real" filesystems to avoid noise (snap, proc, etc.)
    // This is a heuristic and may need adjustments. Like, lots of
    // adjustments...
    if (device.find("/dev/") == 0 && mountpoint.find("/snap") != 0 &&
        mountpoint != "/boot/efi") {
      add_shortest(roots, mountpoint);
    } else if (mountpoint.find("/mnt/") == 0 &&
               type.find("tmpfs") == std::string::npos && device != "none" &&
               device.find("portal") == std::string::npos) {
      add_shortest(roots, mountpoint);
    }
  }
  std::vector<std::filesystem::path> m_roots;
  for (auto el : roots) {
    m_roots.emplace_back(el.second);
  }
  return m_roots;
}

export std::string get_home_dir() {
  const char* home = std::getenv("HOME");
  if (home != nullptr) {
    return home;
  }
  struct passwd* pwd = getpwuid(getuid());
  if (pwd != nullptr) {
    return pwd->pw_dir;
  }
  return ""; // Failed to retrieve
}

} // namespace os::tools::file
