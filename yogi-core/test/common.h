#pragma once

#include "../../3rd_party/nlohmann/json.hpp"
#include "../src/objects/detail/branch/branch_info.h"

#include <gtest/gtest.h>
#include <yogi_core.h>
#include <chrono>
#include <vector>
#include <functional>
#include <sstream>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/filesystem.hpp>

using namespace std::chrono_literals;

#define EXPECT_THROW_ERROR(x, ec)            \
  try {                                      \
    x;                                       \
  } catch (const api::Error& err) {          \
    EXPECT_EQ(err.error_code(), ec);         \
  } catch (...) {                            \
    FAIL() << "Wrong exception type thrown"; \
  }

static const nlohmann::json kBranchProps = nlohmann::json::parse(R"raw(
  {
    "advertising_port": 44442,
    "advertising_address": "ff31::8000:2439",
    "advertising_interval": 0.1,
    "timeout": 3.0
  }
)raw");

static const std::chrono::nanoseconds kTimingMargin = 50ms;
static const std::string kAdvAddress = kBranchProps["advertising_address"];
static const unsigned int kAdvPort = kBranchProps["advertising_port"];

static const auto kUdpProtocol =
    boost::asio::ip::make_address(kAdvAddress).is_v4()
        ? boost::asio::ip::udp::v4()
        : boost::asio::ip::udp::v6();
static const auto kTcpProtocol =
    boost::asio::ip::make_address(kAdvAddress).is_v4()
        ? boost::asio::ip::tcp::v4()
        : boost::asio::ip::tcp::v6();

class Test : public ::testing::Test {
 public:
  Test();
  virtual ~Test();
};

class BranchEventRecorder final {
 public:
  BranchEventRecorder(void* context, void* branch);

  nlohmann::json RunContextUntil(int event, const boost::uuids::uuid& uuid,
                                 int ev_res);
  nlohmann::json RunContextUntil(int event, void* branch, int ev_res);

 private:
  void StartAwaitEvent();
  static void Callback(int res, int event, int ev_res, void* userarg);

  struct CallbackData {
    boost::uuids::uuid uuid;
    nlohmann::json json;
    int event;
    int ev_res;
  };

  void* context_;
  void* branch_;
  boost::uuids::uuid uuid_;
  std::vector<char> json_str_;
  std::vector<CallbackData> events_;
};

class FakeBranch final {
 public:
  FakeBranch();

  void Connect(void* branch,
               std::function<void(utils::ByteVector*)> info_changer = {});
  void Accept(std::function<void(utils::ByteVector*)> info_changer = {});
  void Disconnect();
  void Advertise(std::function<void(utils::ByteVector*)> msg_changer = {});

  bool IsConnectedTo(void* branch) const;

 private:
  void Authenticate(std::function<void(utils::ByteVector*)> info_changer);
  void ExchangeAck();

  objects::detail::BranchInfoPtr info_;
  boost::asio::io_context ioc_;
  boost::asio::ip::udp::endpoint udp_ep_;
  boost::asio::ip::udp::socket udp_socket_;
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::ip::tcp::socket tcp_socket_;
};

class TemporaryWorkdirGuard final {
 public:
  TemporaryWorkdirGuard();
  ~TemporaryWorkdirGuard();

 private:
  boost::filesystem::path old_working_dir_;
  boost::filesystem::path temp_path_;
};

struct CommandLine final {
  int argc;
  char** argv;

  CommandLine(std::initializer_list<std::string> args);
  ~CommandLine();

  CommandLine(const CommandLine&) = delete;
  CommandLine& operator=(const CommandLine&) = delete;
};

void SetupLogging(int verbosity);
void* CreateContext();
void RunContextInBackground(void* context);
void* CreateBranch(void* context, const char* name = nullptr,
                   const char* net_name = nullptr,
                   const char* password = nullptr, const char* path = nullptr);
boost::asio::ip::tcp::endpoint GetBranchTcpEndpoint(void* branch);
boost::uuids::uuid GetBranchUuid(void* branch);
nlohmann::json GetBranchInfo(void* branch);
void CheckJsonElementsAreEqual(const nlohmann::json& a, const nlohmann::json& b,
                               const std::string& key);
std::map<boost::uuids::uuid, nlohmann::json> GetConnectedBranches(void* branch);

template <typename T = std::string>
T GetBranchProperty(void* branch, const char* property) {
  return GetBranchInfo(branch)[property].get<T>();
}

std::string ReadFile(const std::string& filename);

std::ostream& operator<<(std::ostream& os, const std::chrono::nanoseconds& dur);
std::ostream& operator<<(std::ostream& os,
                         const std::chrono::microseconds& dur);
std::ostream& operator<<(std::ostream& os,
                         const std::chrono::milliseconds& dur);
std::ostream& operator<<(std::ostream& os, const std::chrono::seconds& dur);
