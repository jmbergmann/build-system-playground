#pragma once

#include "../../../config.h"
#include "advertising_receiver.h"
#include "advertising_sender.h"
#include "branch_connection.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/functional/hash.hpp>
#include <unordered_set>
#include <unordered_map>
#include <mutex>

namespace objects {
namespace detail {

class ConnectionManager final
    : public std::enable_shared_from_this<ConnectionManager> {
 public:
  typedef std::function<void(const api::Error&, BranchConnectionPtr)>
      ConnectionChangedHandler;
  typedef std::vector<std::pair<boost::uuids::uuid, std::string>>
      BranchInfoStringsList;

  ConnectionManager(ContextPtr context,
                    const boost::asio::ip::udp::endpoint& adv_ep,
                    ConnectionChangedHandler connection_changed_handler);

  const boost::asio::ip::udp::endpoint& GetAdvertisingEndpoint() const {
    return adv_sender_->GetEndpoint();
  }

  boost::asio::ip::tcp::endpoint GetTcpServerEndpoint() const {
    return acceptor_.local_endpoint();
  }

  void Start(BranchInfoPtr info);

  BranchInfoStringsList MakeConnectedBranchesInfoStrings() const;

 private:
  typedef std::unordered_set<boost::uuids::uuid,
                             boost::hash<boost::uuids::uuid>>
      UuidSet;

  typedef std::unordered_map<boost::uuids::uuid, detail::BranchConnectionPtr,
                             boost::hash<boost::uuids::uuid>>
      ConnectionsMap;

  void SetupAcceptor(const boost::asio::ip::tcp& protocol);
  void StartAccept();
  void OnAcceptFinished(const api::Error& err, utils::TimedTcpSocketPtr socket);
  void OnAdvertisementReceived(const boost::uuids::uuid& uuid,
                               const boost::asio::ip::tcp::endpoint& ep);
  void OnConnectFinished(const api::Error& err, const boost::uuids::uuid& uuid,
                         const boost::asio::ip::tcp::endpoint& ep,
                         utils::TimedTcpSocketPtr socket);
  void StartExchangeBranchInfo(utils::TimedTcpSocketPtr socket,
                               bool origin_is_tcp_server);
  void OnExchangeBranchInfoFinished(const api::Error& err,
                                    BranchConnectionPtr connection,
                                    bool origin_is_tcp_server);
  utils::TimedTcpSocketPtr MakeSocket();

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const std::string password_;
  const ConnectionChangedHandler connection_changed_handler_;
  const detail::AdvertisingSenderPtr adv_sender_;
  const detail::AdvertisingReceiverPtr adv_receiver_;

  boost::asio::ip::tcp::acceptor acceptor_;
  BranchInfoPtr info_;
  ConnectionsMap connections_;
  mutable std::mutex connections_mutex_;
  UuidSet blacklisted_uuids_;
};

typedef std::shared_ptr<ConnectionManager> ConnectionManagerPtr;

}  // namespace detail
}  // namespace objects
