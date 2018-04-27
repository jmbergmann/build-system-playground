#include "branch.h"
#include "../api/constants.h"
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
      adv_address_(adv_address),
      adv_port_(adv_port),
      adv_interval_(adv_interval),
      start_time_(utils::GetCurrentUtcTime()),
      adv_tx_endpoint_(boost::asio::ip::make_address(adv_address_), adv_port_),
      adv_tx_socket_(context_->IoContext(), adv_tx_endpoint_.protocol()),
      adv_tx_timer_(context->IoContext()),
      acceptor_(context_->IoContext()) {
  SetupAcceptor();
  adv_message_ = MakeAdvertisingMessage();
}

void Branch::Start() { SendAdvertisement(); }

std::string Branch::MakeInfo() const {
  boost::property_tree::ptree pt;
  pt.put("uuid", boost::uuids::to_string(uuid_));
  pt.put("name", name_);
  pt.put("description", description_);
  pt.put("net_name", net_name_);
  pt.put("path", path_);
  pt.put("hostname", utils::GetHostname());
  pt.put("pid", utils::GetPid());
  pt.put("advertising_address", adv_address_);
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
  auto protocol = adv_tx_endpoint_.protocol() == boost::asio::ip::udp::v4()
                      ? boost::asio::ip::tcp::v4()
                      : boost::asio::ip::tcp::v6();
  boost::asio::ip::tcp::endpoint ep(protocol, 0);

  acceptor_.open(ep.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(ep);
  acceptor_.listen();
}

void Branch::SendAdvertisement() {
  auto weak_self = this->weak_from_this();
  adv_tx_socket_.async_send_to(
      boost::asio::buffer(adv_message_), adv_tx_endpoint_,
      [weak_self](auto ec, auto) {
        auto self = std::static_pointer_cast<Branch>(weak_self.lock());
        if (!self) {
          return;
        }

        if (!ec) {
          self->StartAdvertisingTimer();
        } else if (ec != boost::asio::error::operation_aborted) {
          // TODO: logging error
        }
      });
}

void Branch::StartAdvertisingTimer() {
  adv_tx_timer_.expires_after(adv_interval_);

  auto weak_self = this->weak_from_this();
  adv_tx_timer_.async_wait([weak_self](auto ec) {
    auto self = std::static_pointer_cast<Branch>(weak_self.lock());
    if (!self) {
      return;
    }

    if (!ec) {
      self->SendAdvertisement();
    } else if (ec != boost::asio::error::operation_aborted) {
      // TODO: logging error
    }
  });
}

}  // namespace objects
