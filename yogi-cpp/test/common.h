
#include <gtest/gtest.h>
#include <yogi.h>

#define CHECK_ENUM_ELEMENT(enum, element, macro)            \
  EXPECT_EQ(static_cast<int>(yogi::enum ::element), macro); \
  EXPECT_EQ(yogi::ToString(yogi::enum ::element), #element)
