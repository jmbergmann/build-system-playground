#include <gtest/gtest.h>
#include "../src/constants.h"

template <typename T>
void check_constant(int constant, T value) {

}

TEST(ConstantsTest, GetConstant) {
  int num = -1;
  YOGI_GetConstant(&num, YOGI_CONST_VERSION_MAJOR);
  EXPECT_NE(num, -1);

  EXPECT_EQ(YOGI_GetConstant(nullptr, YOGI_CONST_VERSION_MAJOR),
            YOGI_ERR_INVALID_PARAM);

  EXPECT_EQ(YOGI_GetConstant(&num, -9999), YOGI_ERR_INVALID_PARAM);
}

TEST(ConstantsTest, GetEachConstant) {
  using namespace constants;

  check_constant(YOGI_CONST_VERSION_NUMBER,       kVersionNumber);
  check_constant(YOGI_CONST_VERSION_MAJOR,        kVersionMajor);
  check_constant(YOGI_CONST_VERSION_MINOR,        kVersionMinor);
  check_constant(YOGI_CONST_VERSION_PATCH,        kVersionPatch);
  check_constant(YOGI_CONST_DEFAULT_ADV_PORT,     kDefaultAdvPort);
  check_constant(YOGI_CONST_DEFAULT_ADV_INTERVAL, kDefaultAdvInterval);
}
