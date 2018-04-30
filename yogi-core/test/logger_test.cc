#include <gtest/gtest.h>
#include <yogi_core.h>

#include "../src/utils/system.h"

#include <boost/algorithm/string.hpp>
#include <time.h>
#include <regex>

class LoggerTest : public ::testing::Test {
 protected:
  struct Entry {
    int severity;
    long long timestamp;
    int tid;
    std::string file;
    int line;
    std::string component;
    std::string msg;
  };

  static void Hook(int severity, long long timestamp, int tid, const char* file,
                   int line, const char* component, const char* msg,
                   void* userarg) {
    auto& entries = *static_cast<std::vector<Entry>*>(userarg);
    entries.push_back({severity, timestamp, tid, file, line, component, msg});
  }

  virtual void SetUp() override {
    logger_ = nullptr;
    int res = YOGI_LoggerCreate(&logger_, "My.Component");
    ASSERT_EQ(res, YOGI_OK);
    ASSERT_NE(logger_, nullptr);

    res = YOGI_LogToHook(YOGI_VB_TRACE, &LoggerTest::Hook, &entries_);
    ASSERT_EQ(res, YOGI_OK);
  }

  virtual void TearDown() override {
    int res = YOGI_DestroyAll();
    ASSERT_EQ(res, YOGI_OK);

    YOGI_LogToConsole(YOGI_VB_NONE, 0, 0, nullptr, nullptr);
    YOGI_LogToHook(YOGI_VB_NONE, nullptr, nullptr);
    YOGI_LogToFile(YOGI_VB_NONE, nullptr, nullptr, nullptr);
  }

  void* logger_;
  std::vector<Entry> entries_;
};

TEST_F(LoggerTest, LogToConsole) {
  const char* timefmt = "_%Y_%m_%d_%F_%H_%M_%S_%T_%3_%6_%9_";
  const char* fmt = "_$t_$P_$T_$s_$m_$f_$l_$c_$<_$>_$$_";

  std::string re = "^__####_##_##_####-##-##_##_##_##_##:##:##_###_###_###__";
  boost::replace_all(re, "#", "\\d");
  re += std::to_string(utils::GetProcessId()) + "_";
  re += std::to_string(utils::GetCurrentThreadId()) + "_";
  re += "ERR_Hello_myfile\\.cc_123_My\\.Component___\\$";
  re += "$";

  std::smatch m;

  // Test stdout
  int res = YOGI_LogToConsole(YOGI_VB_TRACE, YOGI_ST_STDOUT, YOGI_FALSE,
                              timefmt, fmt);
  ASSERT_EQ(res, YOGI_OK);

  ::testing::internal::CaptureStdout();
  YOGI_LoggerLog(logger_, YOGI_VB_ERROR, "myfile.cc", 123, "Hello");
  auto text = ::testing::internal::GetCapturedStdout();

  ASSERT_TRUE(text.back() == '\n');
  text.pop_back();
  if (text.back() == '\r') text.pop_back();
  EXPECT_TRUE(std::regex_match(text, m, std::regex(re))) << text;

  // Test stderr
  res = YOGI_LogToConsole(YOGI_VB_TRACE, YOGI_ST_STDERR, YOGI_FALSE, timefmt,
                          fmt);
  ASSERT_EQ(res, YOGI_OK);

  ::testing::internal::CaptureStderr();
  YOGI_LoggerLog(logger_, YOGI_VB_ERROR, "myfile.cc", 123, "Hello");
  text = ::testing::internal::GetCapturedStderr();

  ASSERT_TRUE(text.back() == '\n');
  text.pop_back();
  if (text.back() == '\r') text.pop_back();
  EXPECT_TRUE(std::regex_match(text, m, std::regex(re))) << text;
}

TEST_F(LoggerTest, Colours) {
  // This is not easily testable so we just print a log message for each
  // severity to show the different colours
  YOGI_LoggerSetVerbosity(logger_, YOGI_VB_TRACE);

  int streams[] = {YOGI_ST_STDOUT, YOGI_ST_STDERR};
  for (int stream : streams) {
    YOGI_LogToConsole(YOGI_VB_TRACE, stream, YOGI_TRUE, nullptr, nullptr);

    YOGI_LoggerLog(logger_, YOGI_VB_FATAL, "myfile.cc", 123, "Hello");
    YOGI_LoggerLog(logger_, YOGI_VB_ERROR, "myfile.cc", 123, "Hello");
    YOGI_LoggerLog(logger_, YOGI_VB_WARNING, "myfile.cc", 123, "Hello");
    YOGI_LoggerLog(logger_, YOGI_VB_INFO, "myfile.cc", 123, "Hello");
    YOGI_LoggerLog(logger_, YOGI_VB_DEBUG, "myfile.cc", 123, "Hello");
    YOGI_LoggerLog(logger_, YOGI_VB_TRACE, "myfile.cc", 123, "Hello");
  }
}

TEST_F(LoggerTest, LogToHook) {
  YOGI_LoggerLog(logger_, YOGI_VB_ERROR, "myfile.cc", 123, "Hello");
  ASSERT_FALSE(entries_.empty());

  auto& entry = entries_.back();
  EXPECT_EQ(entry.severity, YOGI_VB_ERROR);
  EXPECT_GT(entry.timestamp, time(nullptr) * 1000000000LL);
  EXPECT_EQ(entry.tid, utils::GetCurrentThreadId());
  EXPECT_EQ(entry.file, "myfile.cc");
  EXPECT_EQ(entry.line, 123);
  EXPECT_EQ(entry.component, "My.Component");
  EXPECT_EQ(entry.msg, "Hello");
}

TEST_F(LoggerTest, LogToFile) {}

TEST_F(LoggerTest, SetVerbosity) {
  int res = YOGI_LoggerSetVerbosity(logger_, YOGI_VB_ERROR);
  EXPECT_EQ(res, YOGI_OK);

  YOGI_LoggerLog(logger_, YOGI_VB_FATAL, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger_, YOGI_VB_ERROR, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger_, YOGI_VB_WARNING, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger_, YOGI_VB_INFO, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger_, YOGI_VB_DEBUG, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger_, YOGI_VB_TRACE, "myfile.cc", 123, "Hello");

  ASSERT_EQ(entries_.size(), 2);
  EXPECT_EQ(entries_[0].severity, YOGI_VB_FATAL);
  EXPECT_EQ(entries_[1].severity, YOGI_VB_ERROR);
}

TEST_F(LoggerTest, SetComponentsVerbosity) {
  void* logger2;
  int res = YOGI_LoggerCreate(&logger2, "Another Logger");
  ASSERT_EQ(res, YOGI_OK);

  YOGI_LoggerSetVerbosity(logger_, YOGI_VB_INFO);
  YOGI_LoggerSetVerbosity(logger2, YOGI_VB_INFO);

  int count = -1;
  res = YOGI_LoggerSetComponentsVerbosity("My\\..*", YOGI_VB_ERROR, &count);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(count, 1);

  YOGI_LoggerLog(logger_, YOGI_VB_INFO, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger2, YOGI_VB_INFO, "myfile.cc", 123, "Hello");

  ASSERT_EQ(entries_.size(), 1);
  EXPECT_EQ(entries_.back().component, "Another Logger");
}

TEST_F(LoggerTest, Log) {
  int res = YOGI_LoggerLog(logger_, YOGI_VB_FATAL, "myfile.cc", 123, "Hello");
  EXPECT_EQ(res, YOGI_OK);

  res = YOGI_LoggerLog(nullptr, YOGI_VB_TRACE, "myfile.cc", 123, "Hello");
  EXPECT_EQ(res, YOGI_OK);
}

TEST_F(LoggerTest, DefaultLoggerVerbosity) {
  YOGI_LoggerLog(logger_, YOGI_VB_INFO, "myfile.cc", 123, "Hello");
  YOGI_LoggerLog(logger_, YOGI_VB_DEBUG, "myfile.cc", 123, "Hello");

  ASSERT_EQ(entries_.size(), 1);
  EXPECT_EQ(entries_.back().severity, YOGI_VB_INFO);
}

TEST_F(LoggerTest, AppLogger) {
  YOGI_LoggerLog(nullptr, YOGI_VB_FATAL, "myfile.cc", 123, "Hello");
  ASSERT_FALSE(entries_.empty());

  auto& entry = entries_.back();
  EXPECT_EQ(entry.component, "App");
}
