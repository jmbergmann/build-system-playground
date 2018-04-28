#include "branch.h"
#include "../api/constants.h"
#include "../api/error.h"
#include "../utils/system.h"
#include "../utils/time.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/endian/arithmetic.hpp>
#include <sstream>

namespace objects {

Branch::Branch(ContextPtr context, std::string name, std::string description,
               std::string net_name, std::string password, std::string path,
               std::string adv_address, int adv_port,
               std::chrono::milliseconds adv_interval)
    : context_(context),
      uuid_(boost::uuids::random_generator()()),
      name_(name),
      description_(description),
      net_name_(net_name),
      password_(password),
      path_(path),
      adv_ep_(boost::asio::ip::address::from_string(adv_address),
              static_cast<unsigned short>(adv_port)),
      adv_interval_(adv_interval),
      start_time_(utils::GetCurrentUtcTime()),
      adv_tx_socket_(context_->IoContext()),
      adv_tx_timer_(context->IoContext()),
      adv_rx_socket_(context_->IoContext()),
      acceptor_(context_->IoContext()) {
  SetupAcceptor();
  SetupAdvertising();
}

void Branch::Start() {
  adv_sender_->Start();
  ReceiveAdvertisement();
}

std::string Branch::MakeInfo() const {
  boost::property_tree::ptree pt;
  pt.put("uuid", boost::uuids::to_string(uuid_));
  pt.put("name", name_);
  pt.put("description", description_);
  pt.put("net_name", net_name_);
  pt.put("path", path_);
  pt.put("hostname", utils::GetHostname());
  pt.put("pid", utils::GetPid());
  pt.put("advertising_address", adv_ep_.address().to_string());
  pt.put("advertising_port", adv_ep_.port());
  pt.put("advertising_interval", (float)adv_interval_.count() / 1000.0f);
  pt.put("tcp_server_port", acceptor_.local_endpoint().port());
  pt.put("start_time", utils::TimeToJavaScriptString(start_time_));
  pt.put("active_connections", 0);  // TODO

  std::stringstream oss;
  boost::property_tree::json_parser::write_json(oss, pt);
  return oss.str();
}

void Branch::ForeachDiscoveredBranch(
    const std::function<void(const boost::uuids::uuid&)>& fn) const {}

void Branch::ForeachDiscoveredBranch(
    const std::function<void(const boost::uuids::uuid&, std::string)>& fn)
    const {}

void Branch::SetupAcceptor() {
  boost::system::error_code ec;

  auto protocol = adv_ep_.protocol() == boost::asio::ip::udp::v4()
                      ? boost::asio::ip::tcp::v4()
                      : boost::asio::ip::tcp::v6();
  auto ep = boost::asio::ip::tcp::endpoint(protocol, 0);

  acceptor_.open(ep.protocol(), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);
  }

  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);
  }

  acceptor_.bind(ep, ec);
  if (ec) {
    throw api::Error(YOGI_ERR_BIND_SOCKET_FAILED);
  }

  acceptor_.listen(acceptor_.max_listen_connections, ec);
  if (ec) {
    throw api::Error(YOGI_ERR_LISTEN_SOCKET_FAILED);
  }
}

void Branch::SetupAdvertising() {
  adv_sender_ = std::make_shared<detail::AdvertisingSender>(
      context_, adv_ep_, adv_interval_, uuid_, acceptor_.local_endpoint());

  boost::system::error_code ec;

  adv_rx_buffer_.resize(adv_sender_->GetMessageSize() + 1);

  adv_rx_socket_.open(adv_ep_.protocol(), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);
  }

  adv_rx_socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true),
                            ec);
  if (ec) {
    throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);
  }

  adv_rx_socket_.bind(
      boost::asio::ip::udp::endpoint(adv_ep_.protocol(), adv_ep_.port()), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_BIND_SOCKET_FAILED);
  }

  adv_rx_socket_.set_option(
      boost::asio::ip::multicast::join_group(adv_ep_.address()), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);
  }
}

void Branch::ReceiveAdvertisement() {
  auto weak_self = this->MakeWeakPtr();
  adv_rx_socket_.async_receive_from(
      boost::asio::buffer(adv_rx_buffer_), adv_rx_sender_ep_,
      [weak_self](auto ec, auto size) {
        auto self = weak_self.lock();
        if (!self) return;

        if (!ec) {
          if (size == self->adv_tx_message_.size()) {
            self->HandleReceivedAdvertisement();
            self->ReceiveAdvertisement();
          } else {
            // TODO: logging error (unexpected adv size received)
          }
        } else if (ec != boost::asio::error::operation_aborted) {
          // TODO: logging error
        }
      });
}

void Branch::HandleReceivedAdvertisement() {
  printf("COOOOOOOOOOOOOOOOOOOOOL\n");
}

}  // namespace objects
