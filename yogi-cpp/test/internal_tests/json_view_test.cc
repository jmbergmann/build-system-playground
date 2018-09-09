#include "../common.h"

using namespace yogi::internal;

TEST(JsonViewTest, Default) {
  EXPECT_EQ(static_cast<const char*>(JsonView()), nullptr);
}

TEST(JsonViewTest, ConstCharString) {
  const char* s = "Hello";
  EXPECT_EQ(static_cast<const char*>(JsonView(s)), s);
}

TEST(JsonViewTest, StdString) {
  std::string s = "Hello";
  EXPECT_EQ(static_cast<const char*>(JsonView(s)), s.c_str());
}

TEST(JsonViewTest, JsonObject) {
  yogi::Json json = {12345};
  EXPECT_EQ(json.dump(), static_cast<const char*>(JsonView(json)));
}
