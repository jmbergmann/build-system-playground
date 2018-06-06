#pragma once

#include "../../3rd_party/json/json.hpp"

#include <gtest/gtest.h>
#include <yogi_core.h>
#include <chrono>
#include <vector>
#include <tuple>
#include <mutex>
#include <condition_variable>
#include <boost/asio/ip/tcp.hpp>
#include <boost/uuid/uuid.hpp>

using namespace std::chrono_literals;

class Test : public ::testing::Test {
 public:
  Test();
  virtual ~Test();
};

class BranchEventObserver final {
 public:
  BranchEventObserver(void* branch);
  ~BranchEventObserver();

  nlohmann::json Wait(int event, const boost::uuids::uuid& uuid, int ev_res);
  nlohmann::json Wait(int event, void* branch, int ev_res);

 private:
  static void Callback(int res, int event, int ev_res, void* userarg);

  void StartObserve();

  struct CallbackData {
    boost::uuids::uuid uuid;
    nlohmann::json json;
    int event;
    int ev_res;
  };

  void* branch_;
  std::mutex mutex_;
  std::condition_variable cv_;
  boost::uuids::uuid uuid_;
  std::vector<char> json_str_;
  std::vector<CallbackData> events_;
};

static const int kAdvPort = 44442;
static const char* kAdvAddress = "ff31::8000:2439";
static const std::chrono::nanoseconds kAdvInterval = 100ms;
static const std::chrono::nanoseconds kConnTimeout = 3000ms;
static const std::chrono::nanoseconds kTimingMargin = 50ms;

void SetupLogging(int verbosity);
void* CreateContext();
void RunContextInBackground(void* context);
void* CreateBranch(void* context, const char* name = nullptr,
                   const char* net_name = nullptr,
                   const char* password = nullptr);
boost::asio::ip::tcp::endpoint GetBranchTcpEndpoint(void* branch);
boost::uuids::uuid GetBranchUuid(void* branch);
void WaitForBranchEvents(
    void* branch, std::vector<std::tuple<int, boost::uuids::uuid, int>> events);
