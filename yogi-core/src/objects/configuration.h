#pragma once

#include "../config.h"
#include "../api/object.h"
#include "../api/error.h"
#include "../utils/types.h"
#include "../../../3rd_party/json/json.hpp"
#include "detail/configuration/command_line_parser.h"
#include "logger.h"

#include <mutex>

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

  using CommandLineOptions = detail::CommandLineParser::CommandLineOptions;

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

}  // namespace objects
