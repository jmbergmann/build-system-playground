#pragma once

#include "../../3rd_party/json/json.hpp"

#include <gtest/gtest.h>
#include <yogi_core.h>
#include <chrono>
#include <vector>
#include <boost/asio/ip/tcp.hpp>
#include <boost/uuid/uuid.hpp>

using namespace std::chrono_literals;

static const int kAdvPort = 44442;
static const char* kAdvAddress = "ff31::8000:2439";
static const std::chrono::nanoseconds kAdvInterval = 100ms;
static const std::chrono::nanoseconds kConnTimeout = 3000ms;
static const std::chrono::nanoseconds kTimingMargin = 50ms;

class Test : public ::testing::Test {
 public:
  Test();
  virtual ~Test();
};

class BranchEventRecorder final {
 public:
  BranchEventRecorder(void* context, void* branch);

  nlohmann::json RunContextUntil(int event, const boost::uuids::uuid& uuid, int ev_res);
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
  void Connect(void* branch);
  void Disconnect();
  void Advertise();
  void Accept();
};

void SetupLogging(int verbosity);
void* CreateContext();
void RunContextInBackground(void* context);
void* CreateBranch(void* context, const char* name = nullptr,
                   const char* net_name = nullptr,
                   const char* password = nullptr,
                   const char* path = nullptr);
boost::asio::ip::tcp::endpoint GetBranchTcpEndpoint(void* branch);
boost::uuids::uuid GetBranchUuid(void* branch);
nlohmann::json GetBranchInfo(void* branch);
void CheckJsonElementsAreEqual(const nlohmann::json& a, const nlohmann::json& b,
                               const std::string& key);

template <typename T = std::string>
T GetBranchProperty(void* branch, const char* property) {
  return GetBranchInfo(branch)[property].get<T>();
}
