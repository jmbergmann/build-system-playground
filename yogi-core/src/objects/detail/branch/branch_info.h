#pragma once

#include "../../../config.h"
#include "../../../utils/socket.h"
#include "../../../utils/timestamp.h"
#include "../../../../../3rd_party/json/json.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <mutex>
#include <string>
#include <chrono>
#include <memory>
#include <vector>

namespace objects {
namespace detail {

class BranchInfo {
 public:
  virtual ~BranchInfo() = default;

  virtual nlohmann::json ToJson() const;

  boost::uuids::uuid uuid;
  std::mutex mutex;
  std::string name;
  std::string description;
  std::string net_name;
  std::string path;
  std::string hostname;
  int pid = 0;
  boost::asio::ip::tcp::endpoint tcp_ep;
  utils::Timestamp start_time;
  std::chrono::nanoseconds timeout;
  std::chrono::nanoseconds retry_time;
};

class LocalBranchInfo : public BranchInfo {
 public:
  LocalBranchInfo();

  virtual nlohmann::json ToJson() const override;
  std::vector<char> MakeAdvertisingMessage() const;
  std::vector<char> MakeInfoMessage() const;

  boost::asio::ip::udp::endpoint adv_ep;
  std::chrono::nanoseconds adv_interval;
};

typedef std::shared_ptr<LocalBranchInfo> LocalBranchInfoPtr;

class RemoteBranchInfo : public BranchInfo {
 public:
  virtual nlohmann::json ToJson() const override;

  bool connected = false;
  utils::Timestamp last_connected;
  utils::Timestamp last_disconnected;
  utils::Timestamp last_activity;
  std::string last_error;
  utils::TimedTcpSocketPtr socket;
};

typedef std::shared_ptr<RemoteBranchInfo> RemoteBranchInfoPtr;

}  // namespace detail
}  // namespace objects
