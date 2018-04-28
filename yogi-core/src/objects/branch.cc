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
      adv_address_(boost::asio::ip::address::from_string(adv_address)),
      adv_port_(adv_port),
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
  SendAdvertisement();
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
  pt.put("advertising_address", adv_address_.to_string());
  pt.put("advertising_port", adv_port_);
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

std::vector<char> Branch::MakeAdvertisingMessage() const {
  std::vector<char> vec = {'Y', 'O', 'G', 'I', 0};
  vec.push_back(api::kVersionMajor);
  vec.push_back(api::kVersionMinor);
  vec.insert(vec.end(), uuid_.begin(), uuid_.end());
  boost::endian::big_uint16_t port = acceptor_.local_endpoint().port();
  vec.resize(vec.size() + 2);
  std::memcpy(vec.data() + vec.size() - 2, &port, 2);

  return vec;
}

void Branch::SetupAcceptor() {
  boost::system::error_code ec;

  auto ep = MakeTcpEndpoint(0);
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
  boost::system::error_code ec;

  adv_tx_endpoint_ = MakeUdpEndpoint(adv_port_);
  adv_tx_endpoint_.address(adv_address_);

  adv_tx_socket_.open(adv_tx_endpoint_.protocol(), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);
  }

  adv_tx_message_ = MakeAdvertisingMessage();

  adv_rx_buffer_.resize(adv_tx_message_.size() + 1);

  auto ep = MakeUdpEndpoint(adv_port_);
  adv_rx_socket_.open(ep.protocol(), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);
  }

  adv_rx_socket_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true),
                            ec);
  if (ec) {
    throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);
  }

  adv_rx_socket_.bind(ep, ec);
  if (ec) {
    throw api::Error(YOGI_ERR_BIND_SOCKET_FAILED);
  }

  adv_rx_socket_.set_option(
      boost::asio::ip::multicast::join_group(adv_address_), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);
  }
}

void Branch::SendAdvertisement() {
  auto weak_self = this->MakeWeakPtr();
  adv_tx_socket_.async_send_to(
      boost::asio::buffer(adv_tx_message_), adv_tx_endpoint_,
      [weak_self](auto ec, auto) {
        auto self = weak_self.lock();
        if (!self) return;

        if (!ec) {
          self->StartAdvertisingTimer();
        } else if (ec != boost::asio::error::operation_aborted) {
          // TODO: logging error
        }
      });
}

void Branch::StartAdvertisingTimer() {
  adv_tx_timer_.expires_after(adv_interval_);

  auto weak_self = this->MakeWeakPtr();
  adv_tx_timer_.async_wait([weak_self](auto ec) {
    auto self = weak_self.lock();
    if (!self) return;

    if (!ec) {
      self->SendAdvertisement();
    } else if (ec != boost::asio::error::operation_aborted) {
      // TODO: logging error
    }
  });
}

void Branch::ReceiveAdvertisement() {
  auto weak_self = this->MakeWeakPtr();
  adv_rx_socket_.async_receive_from(boost::asio::buffer(adv_rx_buffer_), adv_rx_sender_ep_, [weak_self](auto ec, auto size) {
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

boost::asio::ip::tcp::endpoint Branch::MakeTcpEndpoint(int port) {
  auto protocol = adv_address_.is_v4() ? boost::asio::ip::tcp::v4()
                                       : boost::asio::ip::tcp::v6();
  return boost::asio::ip::tcp::endpoint(protocol,
                                        static_cast<unsigned short>(port));
}

boost::asio::ip::udp::endpoint Branch::MakeUdpEndpoint(int port) {
  auto protocol = adv_address_.is_v4() ? boost::asio::ip::udp::v4()
                                       : boost::asio::ip::udp::v6();
  return boost::asio::ip::udp::endpoint(protocol,
                                        static_cast<unsigned short>(port));
}

}  // namespace objects
