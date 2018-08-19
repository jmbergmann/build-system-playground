#include "../common.h"

#include <yogi_core.h>

class ConfigurationTest : public ::testing::Test {};

TEST_F(ConfigurationTest, ConfigurationFlags) {
  // clang-format off
  CHECK_ENUM_ELEMENT(ConfigurationFlags, kNone,             YOGI_CFG_NONE);
  CHECK_ENUM_ELEMENT(ConfigurationFlags, kDisableVariables, YOGI_CFG_DISABLE_VARIABLES);
  CHECK_ENUM_ELEMENT(ConfigurationFlags, kMutableCmdLine,   YOGI_CFG_MUTABLE_CMD_LINE);
  // clang-format on

  auto flags = yogi::ConfigurationFlags::kNone;
  EXPECT_EQ(yogi::ToString(flags), "kNone");
  flags = flags | yogi::ConfigurationFlags::kDisableVariables;
  EXPECT_EQ(yogi::ToString(flags), "kDisableVariables");
  flags |= yogi::ConfigurationFlags::kMutableCmdLine;
  EXPECT_EQ(yogi::ToString(flags), "kDisableVariables | kMutableCmdLine");
}

TEST_F(ConfigurationTest, CommandLineOptions) {
  // clang-format off
  CHECK_ENUM_ELEMENT(CommandLineOptions, kNone,              YOGI_CLO_NONE);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kLogging,           YOGI_CLO_LOGGING);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchName,        YOGI_CLO_BRANCH_NAME);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchDescription, YOGI_CLO_BRANCH_DESCRIPTION);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchNetwork,     YOGI_CLO_BRANCH_NETWORK);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchPassword,    YOGI_CLO_BRANCH_PASSWORD);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchPath,        YOGI_CLO_BRANCH_PATH);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchAdvAddr,     YOGI_CLO_BRANCH_ADV_ADDR);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchAdvPort,     YOGI_CLO_BRANCH_ADV_PORT);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchAdvInt,      YOGI_CLO_BRANCH_ADV_INT);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchTimeout,     YOGI_CLO_BRANCH_TIMEOUT);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kFiles,             YOGI_CLO_FILES);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kFilesRequired,     YOGI_CLO_FILES_REQUIRED);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kOverrides,         YOGI_CLO_OVERRIDES);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kVariables,         YOGI_CLO_VARIABLES);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kBranchAll,         YOGI_CLO_BRANCH_ALL);
  CHECK_ENUM_ELEMENT(CommandLineOptions, kAll,               YOGI_CLO_ALL);
  // clang-format on

  auto options = yogi::CommandLineOptions::kNone;
  EXPECT_EQ(yogi::ToString(options), "kNone");
  options = options | yogi::CommandLineOptions::kLogging;
  EXPECT_EQ(yogi::ToString(options), "kLogging");
  options |= yogi::CommandLineOptions::kBranchName;
  EXPECT_EQ(yogi::ToString(options), "kLogging | kBranchName");
  options |= yogi::CommandLineOptions::kBranchDescription;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription");
  options |= yogi::CommandLineOptions::kBranchNetwork;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork");
  options |= yogi::CommandLineOptions::kBranchPassword;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork | "
            "kBranchPassword");
  options |= yogi::CommandLineOptions::kBranchPath;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork | "
            "kBranchPassword | kBranchPath");
  options |= yogi::CommandLineOptions::kBranchAdvAddr;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork | "
            "kBranchPassword | kBranchPath | kBranchAdvAddr");
  options |= yogi::CommandLineOptions::kBranchAdvPort;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork | "
            "kBranchPassword | kBranchPath | kBranchAdvAddr | kBranchAdvPort");
  options |= yogi::CommandLineOptions::kBranchAdvInt;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchName | kBranchDescription | kBranchNetwork | "
            "kBranchPassword | kBranchPath | kBranchAdvAddr | kBranchAdvPort | "
            "kBranchAdvInt");
  options |= yogi::CommandLineOptions::kBranchTimeout;
  EXPECT_EQ(yogi::ToString(options), "kLogging | kBranchAll");
  options |= yogi::CommandLineOptions::kFiles;
  EXPECT_EQ(yogi::ToString(options), "kLogging | kBranchAll | kFiles");
  options |= yogi::CommandLineOptions::kFilesRequired;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchAll | kFiles | kFilesRequired");
  options |= yogi::CommandLineOptions::kOverrides;
  EXPECT_EQ(yogi::ToString(options),
            "kLogging | kBranchAll | kFiles | kFilesRequired | kOverrides");
  options |= yogi::CommandLineOptions::kVariables;
  EXPECT_EQ(yogi::ToString(options), "kAll");
}
