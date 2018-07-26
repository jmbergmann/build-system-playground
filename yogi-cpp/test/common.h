
#include <gtest/gtest.h>
#include <yogi.h>
#include <regex>

#define CHECK_ENUM_ELEMENT(enum, element, macro)            \
  EXPECT_EQ(static_cast<int>(yogi::enum ::element), macro); \
  EXPECT_EQ(yogi::ToString(yogi::enum ::element), #element)

namespace {

void CheckStringMatches(std::string s, std::string pattern) {
  std::regex re(pattern);
  std::smatch m;
  EXPECT_TRUE(std::regex_match(s, m, re))
      << "String: " << s << " Pattern: " << pattern;
}

}  // anonymous namespace
