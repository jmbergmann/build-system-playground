#pragma once

#include "../config.h"
#include "context.h"

#include <boost/uuid/uuid.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <vector>
#include <string>
#include <functional>

namespace objects {

class Branch : public api::ExposedObjectT<Branch, api::ObjectType::kBranch> {
 public:
  Branch(ContextPtr context, std::string name, std::string description,
         std::string net_name, std::string password, std::string path,
         std::string adv_address, int adv_port,
         std::chrono::milliseconds adv_interval);

  const boost::uuids::uuid& GetUuid() const {
    return uuid_;
  }

  std::string MakeInfo() const;
  void ForeachDiscoveredBranch(
      const std::function<void (const boost::uuids::uuid&)>& fn) const;

  void ForeachDiscoveredBranch(
      const std::function<void (const boost::uuids::uuid&, std::string)>& fn)
      const;

 private:
  const ContextPtr context_;
  const boost::uuids::uuid uuid_;
  const std::string name_;
  const std::string description_;
  const std::string net_name_;
  const std::string password_;
  const std::string path_;
  const std::string adv_address_;
  const int adv_port_;
  const std::chrono::milliseconds adv_interval_;
  const boost::posix_time::ptime start_time_;

  boost::asio::ip::udp::endpoint adv_endpoint_;
  boost::asio::ip::udp::socket adv_socket_;
  boost::asio::steady_timer adv_timer_;
};

typedef std::shared_ptr<Branch> BranchPtr;

}  // namespace objects
