#pragma once

#include "../config.h"
#include "../api/object.h"
#include "../utils/types.h"
#include "logger.h"
#include "../../../3rd_party/json/json.hpp"

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
    kBranchOptions = YOGI_CLO_BRANCH,
    kFileOptions = YOGI_CLO_FILES,
    kJsonOptions = YOGI_CLO_JSON,
    kVariableOptions = YOGI_CLO_VARIABLES,
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
  static const LoggerPtr logger_;

  const bool variables_supported_;
  const bool mutable_cmdline_;
  mutable std::mutex mutex_;
};

typedef std::shared_ptr<Configuration> ConfigurationPtr;

YOGI_DEFINE_FLAG_OPERATORS(Configuration::ConfigurationFlags);
YOGI_DEFINE_FLAG_OPERATORS(Configuration::CommandLineOptions);

}  // namespace objects
