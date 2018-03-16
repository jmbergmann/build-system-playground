#include <gtest/gtest.h>
#include <yogi_core.h>
#include "../src/constants.h"

template <typename T>
void check_constant(int constant, T value) {

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
