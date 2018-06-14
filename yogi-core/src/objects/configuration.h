#pragma once

#include "../config.h"
#include "../api/object.h"
#include "../api/error.h"
#include "../utils/types.h"
#include "logger.h"
#include "../../../3rd_party/json/json.hpp"

#include <boost/program_options.hpp>
#include <mutex>
#include <string>

namespace objects {

class Configuration
    : public api::ExposedObjectT<Configuration,
                                 api::ObjectType::kConfiguration> {
 public:
  enum ConfigurationFlags {
    kNoFlags = YOGI_CFG_NONE,
    kDisableVariables = YOGI_CFG_DISABLE_VARIABLES,
    kMutableCmdline = YOGI_CFG_MUTABLE_CMDLINE,
    kAllFlags = kDisableVariables | kMutableCmdline,
  };

  enum CommandLineOptions {
    kNoOptions = YOGI_CLO_NONE,
    kLoggingOptions = YOGI_CLO_LOGGING,
    kBranchNameOption = YOGI_CLO_BRANCH_NAME,
    kBranchDescriptionOption = YOGI_CLO_BRANCH_DESCRIPTION,
    kBranchNetworkOption = YOGI_CLO_BRANCH_NETWORK,
    kBranchPasswordOption = YOGI_CLO_BRANCH_PASSWORD,
    kBranchPathOption = YOGI_CLO_BRANCH_PATH,
    kBranchAdvaddrOption = YOGI_CLO_BRANCH_ADVADDR,
    kBranchAdvportOption = YOGI_CLO_BRANCH_ADVPORT,
    kBranchAdvintOption = YOGI_CLO_BRANCH_ADVINT,
    kBranchTimeoutOption = YOGI_CLO_BRANCH_TIMEOUT,
    kFileOption = YOGI_CLO_FILES,
    kFileRequiredOption = YOGI_CLO_FILES_REQUIRED,
    kOverrideOption = YOGI_CLO_OVERRIDES,
    kVariableOption = YOGI_CLO_VARIABLES,
    kAllOptions = YOGI_CLO_ALL,
  };

  Configuration(ConfigurationFlags flags);

  void UpdateFromCommandLine(int argc, const char* const* argv,
                             CommandLineOptions options,
                             std::string* err_description);
  void UpdateFromString(const std::string& json_str,
                        std::string* err_description);
  void UpdateFromFile(const std::string& filename,
                      std::string* err_description);
  std::string Dump(bool resolve_variables) const;
  void WriteToFile(const std::string& filename, bool resolve_variables,
                   int identation_width) const;

 private:
  static void VerifyHelpOption(
      const char* const* argv, const boost::program_options::variables_map& vm,
      const boost::program_options::options_description& visible_options,
      CommandLineOptions options, std::string* err_description);
  static void VerifyHelpLoggingOption(
      const boost::program_options::variables_map& vm,
      std::string* err_description);
  static void VerifyLogConsoleOption(
      const boost::optional<std::string>& log_console,
      std::string* err_description);
  static void VerifyFilesOption(
      const std::vector<std::string>& cfg_file_patterns,
      CommandLineOptions options, std::string* err_description);

  static const LoggerPtr logger_;

  const bool variables_supported_;
  const bool mutable_cmdline_;
  mutable std::mutex mutex_;
};

typedef std::shared_ptr<Configuration> ConfigurationPtr;

YOGI_DEFINE_FLAG_OPERATORS(Configuration::ConfigurationFlags);
YOGI_DEFINE_FLAG_OPERATORS(Configuration::CommandLineOptions);

}  // namespace objects
