#pragma once

#include "../../../config.h"
#include "../../logger.h"
#include "../../../api/error.h"
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
  static constexpr std::size_t GetAdvertisingMessageHeaderSize() { return 7; }
  static std::size_t GetAdvertisingMessageSize();
  static std::size_t GetInfoMessageHeaderSize();
  static api::Error CheckAdvertisingMessageValidity(const std::vector<char>& msg);

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
  std::chrono::nanoseconds adv_interval;

 protected:
  static const LoggerPtr logger_;
};

class LocalBranchInfo : public BranchInfo {
 public:
  LocalBranchInfo();

  virtual nlohmann::json ToJson() const override;
  std::vector<char> MakeAdvertisingMessage() const;
  std::vector<char> MakeInfoMessage() const;

  boost::asio::ip::udp::endpoint adv_ep;
};

typedef std::shared_ptr<LocalBranchInfo> LocalBranchInfoPtr;

class RemoteBranchInfo : public BranchInfo {
 public:
  static std::shared_ptr<RemoteBranchInfo> CreateFromAdvertisingMessage(
      const std::vector<char>& msg, utils::TimedTcpSocketPtr socket);

  bool DeserializeInfoMessageBody(const std::vector<char>& msg);

  virtual nlohmann::json ToJson() const override;

  bool connected = false;
  utils::Timestamp last_connected;
  utils::Timestamp last_disconnected;
  utils::Timestamp last_activity;
  api::Error last_error = api::kSuccess;
  utils::TimedTcpSocketPtr socket;

 private:
  template <typename Field>
  static inline bool DeserializeField(Field* field,
                                      const std::vector<char>& msg,
                                      std::vector<char>::const_iterator* it) {
    if (!utils::Deserialize<Field>(field, msg, it)) {
      YOGI_LOG_ERROR(logger_, "Invalid advertising or branch info message");
      return false;
    }

    return true;
  }
};

typedef std::shared_ptr<RemoteBranchInfo> RemoteBranchInfoPtr;

}  // namespace detail
}  // namespace objects
