/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "system.h"

#include <boost/asio/ip/host_name.hpp>

#ifdef _WIN32
# include <process.h>
#else
# include <sys/types.h>
# include <unistd.h>
# include <sys/syscall.h>
#endif

namespace utils {

std::string GetHostname() {
  boost::system::error_code ec;
  auto hostname = boost::asio::ip::host_name(ec);
  if (ec) {
    hostname = "unknown hostname";
  }

  return hostname;
}

int GetProcessId() {
#ifdef _WIN32
  return _getpid();
#else
  return getpid();
#endif
}

int GetCurrentThreadId() {
#ifdef _WIN32
  auto id = ::GetCurrentThreadId();
#else
  auto id = syscall(SYS_gettid);
#endif

  return static_cast<int>(id);
}

}  // namespace utils
