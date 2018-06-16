#include "../common.h"

namespace fs = boost::filesystem;

class ConfigurationTest : public Test {
 protected:
  virtual void SetUp() override { cfg_ = MakeConfiguration(YOGI_CFG_NONE); }

  void* MakeConfiguration(int flags) {
    void* cfg = nullptr;
    int res = YOGI_ConfigurationCreate(&cfg, flags);
    EXPECT_EQ(res, YOGI_OK);
    EXPECT_NE(cfg, nullptr);

    res = YOGI_ConfigurationUpdateFromJson(
        cfg, "{\"person\": {\"name\": \"Joe\", \"age\": 42}}", nullptr, 0);
    EXPECT_EQ(res, YOGI_OK);

    return cfg;
  }

  nlohmann::json DumpConfiguration(void* cfg) {
    char str[1000];
    int res = YOGI_ConfigurationDump(cfg, str, sizeof(str), YOGI_TRUE, -1);
    if (res == YOGI_ERR_NO_VARIABLE_SUPPORT) {
      res = YOGI_ConfigurationDump(cfg, str, sizeof(str), YOGI_FALSE, -1);
    }
    EXPECT_EQ(res, YOGI_OK);

    return nlohmann::json::parse(str);
  }

  void CheckConfigurationIsOriginal() {
    auto expected =
        nlohmann::json::parse("{\"person\": {\"name\": \"Joe\", \"age\": 42}}");
    auto actual = DumpConfiguration(cfg_);
    EXPECT_TRUE(actual == expected) << "Expected:" << std::endl
                                    << expected.dump(2) << std::endl
                                    << "Actual:" << std::endl
                                    << actual.dump(2) << std::endl;
  }

  void* cfg_;
};

TEST_F(ConfigurationTest, UpdateFromJson) {
  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromJson(
      cfg_, "{\"person\": {\"age\": 10}}", err_desc, sizeof(err_desc));
  EXPECT_EQ(res, YOGI_OK) << err_desc;
  EXPECT_STREQ(err_desc, "");

  auto json = DumpConfiguration(cfg_);
  ASSERT_TRUE(json.count("person")) << json;
  EXPECT_EQ(json["person"].value("name", "NOT FOUND"), "Joe");
  EXPECT_EQ(json["person"].value("age", -1), 10);
}

TEST_F(ConfigurationTest, UpdateFromCorruptJson) {
  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromJson(cfg_, "{\"person\": {\"age\": 10}",
                                             err_desc, sizeof(err_desc));
  EXPECT_EQ(res, YOGI_ERR_PARSING_JSON_FAILED);
  EXPECT_STRNE(err_desc, "");

  CheckConfigurationIsOriginal();
}

TEST_F(ConfigurationTest, UpdateFromCommandLine) {
  TemporaryWorkdirGuard workdir;
  {
    fs::ofstream file("a.json");
    file << "{\"person\": {\"age\": 10}}";
  }

  // clang-format off
  CommandLine cmdline{
    "--name", "My Branch",
    "a.json",
  };
  // clang-format on

  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromCommandLine(cfg_, cmdline.argc,
                                                    cmdline.argv, YOGI_CLO_ALL,
                                                    err_desc, sizeof(err_desc));
  ASSERT_EQ(res, YOGI_OK) << err_desc;
  EXPECT_STREQ(err_desc, "");

  auto json = DumpConfiguration(cfg_);
  ASSERT_TRUE(json.count("person")) << json;
  EXPECT_EQ(json["person"].value("name", "NOT FOUND"), "Joe");
  EXPECT_EQ(json["person"].value("age", -1), 10);
  ASSERT_TRUE(json.count("branch")) << json;
  EXPECT_EQ(json["branch"].value("name", "NOT FOUND"), "My Branch");
}

TEST_F(ConfigurationTest, UpdateFromCommandLineCorruptFile) {
  TemporaryWorkdirGuard workdir;
  {
    fs::ofstream file("a.json");
    file << "{\"person\": {\"age\": 10}";
  }

  // clang-format off
  CommandLine cmdline{
    "--name", "My Branch",
    "a.json",
  };
  // clang-format on

  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromCommandLine(cfg_, cmdline.argc,
                                                    cmdline.argv, YOGI_CLO_ALL,
                                                    err_desc, sizeof(err_desc));
  ASSERT_EQ(res, YOGI_ERR_PARSING_FILE_FAILED);
  EXPECT_STRNE(err_desc, "");

  CheckConfigurationIsOriginal();
}

TEST_F(ConfigurationTest, UpdateFromFile) {
  TemporaryWorkdirGuard workdir;
  {
    fs::ofstream file("a.json");
    file << "{\"person\": {\"age\": 10}}";
  }

  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromFile(cfg_, "a.json", err_desc,
                                             sizeof(err_desc));
  ASSERT_EQ(res, YOGI_OK) << err_desc;
  EXPECT_STREQ(err_desc, "");

  auto json = DumpConfiguration(cfg_);
  ASSERT_TRUE(json.count("person")) << json;
  EXPECT_EQ(json["person"].value("name", "NOT FOUND"), "Joe");
  EXPECT_EQ(json["person"].value("age", -1), 10);
}

TEST_F(ConfigurationTest, UpdateFromCorruptFile) {
  TemporaryWorkdirGuard workdir;
  {
    fs::ofstream file("a.json");
    file << "{\"person\": {\"age\": 10}";
  }

  char err_desc[1000];
  int res = YOGI_ConfigurationUpdateFromFile(cfg_, "a.json", err_desc,
                                             sizeof(err_desc));
  ASSERT_EQ(res, YOGI_ERR_PARSING_FILE_FAILED);
  EXPECT_STRNE(err_desc, "");

  CheckConfigurationIsOriginal();
}

TEST_F(ConfigurationTest, Dump) {}

TEST_F(ConfigurationTest, WriteToFile) { TemporaryWorkdirGuard workdir; }

TEST_F(ConfigurationTest, ImmutableCommandLine) {}

TEST_F(ConfigurationTest, Variables) {}

TEST_F(ConfigurationTest, BadVariables) {}
