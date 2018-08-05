
#include <gtest/gtest.h>
#include <yogi.h>
#include <regex>
#include <sstream>
#include <chrono>

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

std::ostream& operator<< (std::ostream& os, const std::chrono::nanoseconds& ns) {
  return os << ns.count() << "ns";
}

}  // anonymous namespace
