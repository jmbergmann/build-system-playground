#pragma once

#include <gtest/gtest.h>
#include <yogi_core.h>
#include <chrono>
#include <boost/asio/ip/tcp.hpp>

using namespace std::chrono_literals;

class Test : public ::testing::Test {
 public:
  Test();
  virtual ~Test();
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
