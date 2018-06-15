#pragma once

#include "../../../config.h"
#include "../../../api/error.h"
#include "../../../utils/timestamp.h"
#include "../../../utils/types.h"
#include "../../../../../3rd_party/nlohmann/json.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <chrono>
#include <memory>

namespace objects {
namespace detail {

class BranchInfo {
 public:
  enum {
    kAdvertisingMessageSize = 25,
    kInfoMessageHeaderSize = kAdvertisingMessageSize + 4,
  };

  static std::shared_ptr<BranchInfo> CreateLocal(
      std::string name, std::string description, std::string net_name,
      std::string path, const boost::asio::ip::tcp::endpoint& tcp_ep,
      const std::chrono::nanoseconds& timeout,
      const std::chrono::nanoseconds& adv_interval);

  static std::shared_ptr<BranchInfo> CreateFromInfoMessage(
      const utils::ByteVector& info_msg, const boost::asio::ip::address& addr);

  static api::Error DeserializeAdvertisingMessage(
      boost::uuids::uuid* uuid, unsigned short* tcp_port,
      const utils::ByteVector& adv_msg);

  static api::Error DeserializeInfoMessageBodySize(
      std::size_t* body_size, const utils::ByteVector& info_msg_hdr);

  const boost::uuids::uuid& GetUuid() const { return uuid_; }

  const std::string& GetName() const { return name_; }
  const std::string& GetDescription() const { return description_; }
  const std::string& GetNetName() const { return net_name_; }
  const std::string& GetPath() const { return path_; }
  const std::string& GetHostname() const { return hostname_; }

  int GetPid() const { return pid_; }

  const boost::asio::ip::tcp::endpoint& GetTcpEndpoint() const {
    return tcp_ep_;
  }

  const utils::Timestamp& GetStartTime() const { return start_time_; }
  const std::chrono::nanoseconds& GetTimeout() const { return timeout_; }

  const std::chrono::nanoseconds& GetAdvertisingInterval() const {
    return adv_interval_;
  }

  utils::SharedByteVector MakeAdvertisingMessage() const {
    YOGI_ASSERT(adv_msg_);
    return adv_msg_;
  }

  utils::SharedByteVector MakeInfoMessage() const {
    YOGI_ASSERT(info_msg_);
    return info_msg_;
  };

  const nlohmann::json& ToJson() const { return json_; }

 private:
  static api::Error CheckMagicPrefixAndVersion(
      const utils::ByteVector& adv_msg);

  void PopulateMessages();
  void PopulateJson();

  boost::uuids::uuid uuid_;
  std::string name_;
  std::string description_;
  std::string net_name_;
  std::string path_;
  std::string hostname_;
  int pid_;
  boost::asio::ip::tcp::endpoint tcp_ep_;
  utils::Timestamp start_time_;
  std::chrono::nanoseconds timeout_;
  std::chrono::nanoseconds adv_interval_;

  utils::SharedByteVector adv_msg_;
  utils::SharedByteVector info_msg_;
  nlohmann::json json_;
};

typedef std::shared_ptr<BranchInfo> BranchInfoPtr;

}  // namespace detail
}  // namespace objects

// format like this: [6ba7b810-9dad-11d1-80b4-00c04fd430c8]
std::ostream& operator<<(std::ostream& os,
                         const objects::detail::BranchInfoPtr& info);
