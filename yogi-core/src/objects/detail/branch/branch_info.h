#pragma once

#include "../../../config.h"
#include "../../../utils/timestamp.h"

#include <boost/uuid/uuid.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <mutex>
#include <string>
#include <chrono>
#include <memory>

namespace objects {
namespace detail {

struct BranchInfo {
  std::mutex mutex;
  boost::uuids::uuid uuid;
  boost::asio::ip::tcp::endpoint tcp_ep;
  std::string name;
  std::string description;
  std::string net_name;
  std::string path;
  std::string hostname;
  int pid = 0;
  std::chrono::nanoseconds adv_interval;
  bool connected = false;
  utils::Timestamp start_time;
  utils::Timestamp last_connected;
  utils::Timestamp last_disconnected;
  utils::Timestamp last_activity;
  std::string last_error;
};

typedef std::shared_ptr<BranchInfo> BranchInfoPtr;

}  // namespace detail
}  // namespace objects
