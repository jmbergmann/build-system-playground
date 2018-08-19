#pragma once

#include "object.h"
#include "io.h"
#include "internal/flags.h"

#include <memory>

namespace yogi {

YOGI_DEFINE_API_FN(int, YOGI_ConfigurationCreate, (void** config, int flags))
YOGI_DEFINE_API_FN(int, YOGI_ConfigurationUpdateFromCommandLine,
                   (void* config, int argc, const char* const* argv,
                    int options, char* err, int errsize))
YOGI_DEFINE_API_FN(int, YOGI_ConfigurationUpdateFromJson,
                   (void* config, const char* json, char* err, int errsize))
YOGI_DEFINE_API_FN(int, YOGI_ConfigurationUpdateFromFile,
                   (void* config, const char* filename, char* err, int errsize))
YOGI_DEFINE_API_FN(int, YOGI_ConfigurationDump,
                   (void* config, char* json, int jsonsize, int resvars,
                    int indent))
YOGI_DEFINE_API_FN(int, YOGI_ConfigurationWriteToFile,
                   (void* config, const char* filename, int resvars,
                    int indent))

/// Flags used to change a configuration object's behaviour.
enum class ConfigurationFlags {
  /// No flags.
  kNone = 0,

  /// Disables support for variables in the configuration.
  kDisableVariables = (1 << 0),

  /// Makes configuration options given directly on the command line
  /// overridable.
  kMutableCmdLine = (1 << 1),
};

YOGI_DEFINE_FLAG_OPERATORS(ConfigurationFlags)

template <>
inline std::string ToString<ConfigurationFlags>(
    const ConfigurationFlags& flags) {
  switch (flags) {
    YOGI_TO_STRING_ENUM_CASE(ConfigurationFlags, kNone)
    YOGI_TO_STRING_ENUM_CASE(ConfigurationFlags, kDisableVariables)
    YOGI_TO_STRING_ENUM_CASE(ConfigurationFlags, kMutableCmdLine)
  }

  std::string s;
  YOGI_TO_STRING_FLAG_APPENDER(flags, ConfigurationFlags, kDisableVariables)
  YOGI_TO_STRING_FLAG_APPENDER(flags, ConfigurationFlags, kMutableCmdLine)
  return s.substr(3);
}

/// Flags used to adjust how command line options are parsed.
enum class CommandLineOptions {
  /// No options.
  kNone = 0,

  /// Include logging configuration for file logging.
  kLogging = (1 << 0),

  /// Include branch name configuration.
  kBranchName = (1 << 1),

  /// Include branch description configuration.
  kBranchDescription = (1 << 2),

  /// Include network name configuration.
  kBranchNetwork = (1 << 3),

  /// Include network password configuration.
  kBranchPassword = (1 << 4),

  /// Include branch path configuration.
  kBranchPath = (1 << 5),

  /// Include branch advertising address configuration.
  kBranchAdvAddr = (1 << 6),

  /// Include branch advertising port configuration.
  kBranchAdvPort = (1 << 7),

  /// Include branch advertising interval configuration.
  kBranchAdvInt = (1 << 8),

  /// Include branch timeout configuration.
  kBranchTimeout = (1 << 9),

  /// Parse configuration files given on the command line.
  kFiles = (1 << 10),

  /// Same as the Files option but at least one configuration file must be
  /// given.
  kFilesRequired = (1 << 11),

  /// Allow overriding arbitrary configuration sections.
  kOverrides = (1 << 12),

  /// Allow setting variables via a dedicated switch.
  kVariables = (1 << 13),

  /// Combination of all branch flags.
  kBranchAll = kBranchName | kBranchDescription | kBranchNetwork |
               kBranchPassword | kBranchPath | kBranchAdvAddr | kBranchAdvPort |
               kBranchAdvInt | kBranchTimeout,

  /// Combination of all flags.
  kAll =
      kLogging | kBranchAll | kFiles | kFilesRequired | kOverrides | kVariables,
};

YOGI_DEFINE_FLAG_OPERATORS(CommandLineOptions)

template <>
inline std::string ToString<CommandLineOptions>(
    const CommandLineOptions& options) {
  switch (options) {
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kNone)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kLogging)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchName)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchDescription)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchNetwork)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchPassword)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchPath)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchAdvAddr)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchAdvPort)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchAdvInt)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchTimeout)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kFiles)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kFilesRequired)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kOverrides)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kVariables)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kBranchAll)
    YOGI_TO_STRING_ENUM_CASE(CommandLineOptions, kAll)
  }

  std::string s;
  YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kLogging)
  if ((options & CommandLineOptions::kBranchAll) ==
      CommandLineOptions::kBranchAll) {
    YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchAll)
  } else {
    YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchName)
    YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions,
                                 kBranchDescription)
    YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchNetwork)
    YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchPassword)
    YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchPath)
    YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchAdvAddr)
    YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchAdvPort)
    YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchAdvInt)
    YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kBranchTimeout)
  }

  YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kFiles)
  YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kFilesRequired)
  YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kOverrides)
  YOGI_TO_STRING_FLAG_APPENDER(options, CommandLineOptions, kVariables)
  return s.substr(3);
}

class Configuration;
using ConfigurationPtr = std::shared_ptr<Configuration>;

class Configuration : public ObjectT<Configuration> {
 public:
  /// Creates a timer.
  ///
  /// \param context The context to use.
  ///
  /// \returns The created Timer.
  static ConfigurationPtr Create() {
    return ConfigurationPtr(new Configuration());
  }

 private:
  Configuration()
      : ObjectT(internal::CallApiCreate(internal::YOGI_ConfigurationCreate, 0),
                {}) {}
};

}  // namespace yogi
