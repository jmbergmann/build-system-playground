#pragma once

#include "../../../config.h"
#include "../../../utils/socket.h"
#include "../../context.h"
#include "../../logger.h"
#include "branch_info.h"

#include <boost/asio.hpp>
#include <memory>
#include <functional>

namespace objects {
namespace detail {

class TcpServer : public std::enable_shared_from_this<TcpServer> {
 public:
  TcpServer(ContextPtr context, LocalBranchInfoPtr info);

  void Start();

 private:
  void SetupAcceptor();
  void StartAccept();
  void OnAccepted(utils::TimedTcpSocketPtr socket);
  void SendInfo(utils::TimedTcpSocketPtr socket);
  void OnInfoSent(utils::TimedTcpSocketPtr socket);
  void StartReceiveInfoHeader(utils::TimedTcpSocketPtr socket);
  void OnInfoHeaderReceived(utils::TimedTcpSocketPtr socket,
                            const std::vector<char>& buffer);
  void StartReceiveInfoBody(utils::TimedTcpSocketPtr socket);
  void OnInfoBodyReceived(utils::TimedTcpSocketPtr socket,
                          const std::vector<char>& buffer);

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const LocalBranchInfoPtr info_;
  std::vector<char> serialized_info_;
  boost::asio::ip::tcp::acceptor acceptor_;
};

typedef std::shared_ptr<TcpServer> TcpServerPtr;

}  // namespace detail
}  // namespace objects
