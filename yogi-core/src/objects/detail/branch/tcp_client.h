#pragma once

#include "../../../config.h"
#include "../../../api/error.h"
#include "../../../utils/socket.h"
#include "../../context.h"
#include "../../logger.h"
#include "branch_info.h"

#include <boost/asio.hpp>
#include <memory>
#include <functional>

namespace objects {
namespace detail {

class TcpClient : public std::enable_shared_from_this<TcpClient> {
 public:
  TcpClient(ContextPtr context, LocalBranchInfoPtr info);

  void Connect(const boost::asio::ip::tcp::endpoint& ep,
               std::function<void(const api::Error&)>&& handler);

 private:
  void OnConnected(RemoteBranchInfoPtr peer);

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const LocalBranchInfoPtr info_;
};

typedef std::shared_ptr<TcpClient> TcpClientPtr;

}  // namespace detail
}  // namespace objects
