#include "common.h"
#include "../src/api/constants.h"

void check(int constant, int value) {
  int x;
  YOGI_GetConstant(&x, constant);
  EXPECT_EQ(x, value) << "Constant (int): ";
}

void check(int constant, long long value) {
  long long x;
  YOGI_GetConstant(&x, constant);
  EXPECT_EQ(x, value) << "Constant (long long): ";
}

void check(int constant, const char* value) {
  const char* x;
  YOGI_GetConstant(&x, constant);
  EXPECT_STREQ(x, value) << "Constant (const char*): ";
}

TEST(ConstantsTest, GetConstant) {
  int num = -1;
  YOGI_GetConstant(&num, YOGI_CONST_VERSION_MAJOR);
  EXPECT_NE(num, -1);

  int res = YOGI_GetConstant(nullptr, YOGI_CONST_VERSION_MAJOR);
  EXPECT_EQ(res, YOGI_ERR_INVALID_PARAM);

  EXPECT_EQ(YOGI_GetConstant(&num, -9999), YOGI_ERR_INVALID_PARAM);
}

TEST(ConstantsTest, GetEachConstant) {
  using namespace api;

  check(YOGI_CONST_VERSION_NUMBER,                    kVersionNumber);
  check(YOGI_CONST_VERSION_MAJOR,                     kVersionMajor);
  check(YOGI_CONST_VERSION_MINOR,                     kVersionMinor);
  check(YOGI_CONST_VERSION_PATCH,                     kVersionPatch);
  check(YOGI_CONST_DEFAULT_ADV_ADDRESS,               kDefaultAdvAddress);
  check(YOGI_CONST_DEFAULT_ADV_PORT,                  kDefaultAdvPort);
  check(YOGI_CONST_DEFAULT_ADV_INTERVAL,              kDefaultAdvInterval);
  check(YOGI_CONST_DEFAULT_CONNECTION_TIMEOUT,        kDefaultConnectionTimeout);
  check(YOGI_CONST_DEFAULT_BRANCHES_CLEANUP_INTERVAL, kDefaultBranchesCleanupInterval);
  check(YOGI_CONST_DEFAULT_LOGGER_VERBOSITY,          kDefaultLoggerVerbosity);
  check(YOGI_CONST_DEFAULT_LOG_TIME_FORMAT,           kDefaultLogTimeFormat);
  check(YOGI_CONST_DEFAULT_LOG_FORMAT,                kDefaultLogFormat);
}
