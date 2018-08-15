#include "../common.h"

#include <regex>

class FakeObject : public yogi::Object {
 public:
  FakeObject() : yogi::Object(nullptr) {}
};

class ObjectTest : public ::testing::Test {
 protected:
  yogi::Context object_;
};

TEST_F(ObjectTest, Format) {
  std::string s = object_.Format();
  CheckStringMatches(s, "^Context \\[[1-9a-f][0-9a-f]+\\]$");

  s = object_.Format("$T-[$X]");
  CheckStringMatches(s, "^Context-\\[[1-9A-F][0-9A-F]+\\]$");

  FakeObject obj;
  s = obj.Format();
  EXPECT_EQ(s.find("INVALID"), 0);

  s = obj.Format(nullptr, "MOO");
  EXPECT_EQ(s, "MOO");
}

TEST_F(ObjectTest, ToString) {
  CheckStringMatches(object_.ToString(), "^Context \\[[1-9a-f][0-9a-f]+\\]$");
}
