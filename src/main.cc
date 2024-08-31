//
// Copyright 2018 Tomasz Wiszkowski
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <unistd.h>

#include <experimental/filesystem>
#include <fstream>
#include <iostream>

#include "dnsmasq_config.h"
#include "mock/nvram.h"
#include "process_nvram.h"
#include "scoped_service_shutdown.h"
#include "version.h"

namespace fs = std::experimental::filesystem;

namespace {
// Keep the temporary file name 'dnsmasq' without any further extensions.
// Asus' RC expects commands to have very specific names when it confirms
// their running state or when it shuts them down (~"pkill").
constexpr const char kTemporaryName[] = "/tmp/dnsmasq";
constexpr const char kOriginalName[] = "/usr/sbin/dnsmasq";
constexpr const char kDnsMasqHostsPath[] = "/jffs/dnsmasq-surrogate/hosts";
constexpr const char kDnsMasqConfigPath[] = "/etc/dnsmasq.conf";
}  // namespace

// Get path to this executable
std::string getSelfPath() {
  char path[PATH_MAX];
  auto len = readlink("/proc/self/exe", path, sizeof(path) - 1);
  if (len > 0) {
    path[len] = 0;
    return path;
  }
  return "/proc/self/exe";
}

// Install dnsmasq substitute using bind-mounting.
// The replacement will point to us as a surrogate; this gives us time to look
// into Asus' configuration settings and apply our own modifications before
// launching actual dnsmasq.
//
// Restarts dnsmasq service.
//
// Returns 0 on success.
int installSubstitute() {
  {
    std::ofstream marker{kTemporaryName};
    if (!marker.good()) {
      std::clog << "Could not create temporary file: " << kTemporaryName
                << "\n";
      return 1;
    }
    marker.flush();
    marker.close();
  }

  asus::ScopedServiceShutdown dnsmasq("dnsmasq");

  auto res = mount(kOriginalName, kTemporaryName, nullptr, MS_BIND, nullptr);
  if (res) {
    std::clog << "Could not install " << kTemporaryName << ": "
              << strerror(errno) << '\n';
    return 1;
  }

  auto self_path = getSelfPath();
  res = mount(self_path.c_str(), kOriginalName, nullptr, MS_BIND, nullptr);
  if (res) {
    std::clog << "Could not install " << kOriginalName << ": "
              << strerror(errno) << '\n';
    return 1;
  }

  fs::create_directories(kDnsMasqHostsPath);

  return 0;
}

// Remove dnsmasq substitute by unmounting original dnsmasq tool.
//
// Restarts dnsmasq service.
//
// Returns 0 on success.
int removeSubstitute() {
  std::clog << "Removing surrogate...\n";

  asus::ScopedServiceShutdown dnsmasq("dnsmasq");

  auto res = umount2(kOriginalName, MNT_DETACH);
  if (res) {
    std::clog << "Could not uninstall " << kOriginalName << ": "
              << strerror(errno) << '\n';
  }

  res = umount2(kTemporaryName, MNT_DETACH);
  if (res) {
    std::clog << "Could not install " << kTemporaryName << ": "
              << strerror(errno) << '\n';
  }

  return 0;
}

// Print usage to console.
int help(char* const cmd) {
  std::clog << "Usage:\n";
  std::clog << "\t" << cmd
            << " install      -- install surrogate and restart service.\n";
  std::clog << "\t" << cmd
            << " remove       -- remove surrogate and restart service.\n";
  std::clog << "\t" << cmd
            << " showconfig   -- dump resulting config on screen.\n";
  std::clog << "\t" << cmd
            << " version      -- show software version and exit.\n";
  return 1;
}

asus::DnsMasqConfig buildConfig() {
  auto clients =
      asus::ProcessCustomClientList(bcm::nvram_get("custom_clientlist"));
  asus::DnsMasqConfig c;

  std::ifstream cfg(kDnsMasqConfigPath);
  if (cfg.good()) c.Load(cfg);

  c.RewriteHosts(clients);

  for (auto&& d : fs::directory_iterator(kDnsMasqHostsPath)) {
    if (fs::is_regular_file(d.status())) {
      c.AddHostsFile(d.path());
    }
  }

  return c;
}

// Rebuild and save dnsmasq configuration file. Execute this right before
// jumping to actual dnsmasq to supply hostnames in your system.
void rebuildConfig() {
  auto c = buildConfig();

  std::ofstream cfg(kDnsMasqConfigPath);
  if (!cfg.good()) {
    std::clog << "Could not save dnsmasq config file " << kDnsMasqConfigPath
              << '\n';
    return;
  }

  c.Save(cfg);
}

int showConfig() {
  auto c = buildConfig();
  c.Save(std::clog);
  return 0;
}

int version() {
  std::clog << kVersionString << '\n';
  return 0;
}

int main(int argc, char* const argv[]) {
  using namespace std::literals;

  fs::path self_path = fs::current_path();
  self_path.append(argv[0]);

  if (self_path.filename() == "dnsmasq") {
    rebuildConfig();
    execve(kTemporaryName, &argv[1], nullptr);
    std::clog << "Could not start dnsmasq: " << strerror(errno) << "\n";
  } else {
    if (argc == 2) {
      if (argv[1] == "install"sv) return installSubstitute();
      if (argv[1] == "remove"sv) return removeSubstitute();
      if (argv[1] == "version"sv) return version();
      if (argv[1] == "showconfig"sv) return showConfig();
    }
    return help(argv[0]);
  }
}
