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
#include "../api/errors.h"

#include <boost/asio/ip/host_name.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>

#include <memory>
#include <algorithm>
#include <cstdio>

#ifdef _WIN32
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#endif

#ifdef __APPLE__
#include <net/if_dl.h>
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
#if defined(_WIN32)
  auto id = ::GetCurrentThreadId();
#elif defined(__APPLE__)
  std::uint64_t id;
  pthread_threadid_np(NULL, &id);
#else
  auto id = syscall(SYS_gettid);
#endif

  return static_cast<int>(id);
}

std::vector<NetworkInterfaceInfo> GetNetworkInterfaces() {
  std::vector<NetworkInterfaceInfo> ifs;

#ifdef _WIN32
#error \
    "Implement me: https://docs.microsoft.com/en-us/windows/desktop/api/iphlpapi/nf-iphlpapi-getadaptersaddresses"
#else
  std::unique_ptr<ifaddrs, decltype(&freeifaddrs)> if_addr_list(nullptr,
                                                                &freeifaddrs);
  {
    ifaddrs* p;
    if (getifaddrs(&p) == -1) {
      char str[100] = {0};
      strerror_r(errno, str, sizeof(str));
      throw api::DescriptiveError(YOGI_ERR_ENUMERATE_NETWORK_INTERFACES_FAILED)
          << str;
    }
    if_addr_list.reset(p);

    for (auto ifa = if_addr_list.get(); ifa != nullptr; ifa = ifa->ifa_next) {
      auto info = std::find_if(ifs.begin(), ifs.end(), [&](auto& info) {
        return info.name == ifa->ifa_name;
      });

      if (info == ifs.end()) {
        info = ifs.insert(ifs.end(), NetworkInterfaceInfo{});
        info->name = ifa->ifa_name;
        info->identifier = info->name;
      }

      switch (ifa->ifa_addr->sa_family) {
#ifdef __APPLE__
        case AF_LINK: {
          auto addr = reinterpret_cast<sockaddr_dl*>(ifa->ifa_addr);
          auto p =
              reinterpret_cast<unsigned char*>(addr->sdl_data + addr->sdl_nlen);
#else
        case AF_PACKET: {
          auto addr = reinterpret_cast<sockaddr_ll*>(ifa->ifa_addr);
          auto p = addr->sll_addr;
#endif
          if (*p || (*(p + 1)) || (*(p + 2)) || (*(p + 3)) || (*(p + 4)) ||
              (*(p + 5))) {
            char mac[18];
            snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x", *p,
                     *(p + 1), *(p + 2), *(p + 3), *(p + 4), *(p + 5));
            info->mac = mac;
          }

          break;
        }

        case AF_INET:
        case AF_INET6: {
          char str[INET6_ADDRSTRLEN];
          boost::asio::ip::address addr;

          if (ifa->ifa_addr->sa_family == AF_INET) {
            auto sin = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
            if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str))) {
              addr = boost::asio::ip::make_address_v4(str);
            }
          } else {
            auto sin = reinterpret_cast<sockaddr_in6*>(ifa->ifa_addr);
            if (inet_ntop(AF_INET6, &sin->sin6_addr, str, sizeof(str))) {
              auto v6addr = boost::asio::ip::make_address_v6(str);
              if (sin->sin6_scope_id != 0) {  // Ignores ::1 but keeps fe80::1
                v6addr.scope_id(sin->sin6_scope_id);
                addr = v6addr;
              }
            }
          }

          if (!addr.is_unspecified()) {
            info->addresses.push_back(addr);
            if (addr.is_loopback()) {
              info->is_loopback = true;
            }
          }

          break;
        }
      }
    }
  }
#endif

  auto it = ifs.begin();
  while (it != ifs.end()) {
    if (it->mac.empty() && it->addresses.empty()) {
      it = ifs.erase(it);
    } else {
      ++it;
    }
  }

  return ifs;
}

std::vector<utils::NetworkInterfaceInfo> GetFilteredNetworkInterfaces(
    const std::vector<std::string>& adv_if_strings) {
  std::vector<utils::NetworkInterfaceInfo> ifs;

  for (auto& string : adv_if_strings) {
    for (auto& info : GetNetworkInterfaces()) {
      auto string_lower = boost::to_lower_copy(string);
      if (string_lower == "all" || string == info.name || string == info.mac ||
          (string_lower == "localhost" && info.is_loopback)) {
        bool already_exists =
            std::find_if(ifs.begin(), ifs.end(), [&](auto& existing_info) {
              return existing_info.name == info.name;
            }) != ifs.end();
        if (!already_exists) {
          ifs.push_back(info);
        }
      }
    }
  }

  return ifs;
}

}  // namespace utils
