#include "configuration.h"

namespace objects {

Configuration::Configuration(ConfigurationFlags flags)
    : variables_supported_(!(flags & kDisableVariables)),
      mutable_cmdline_(!(flags & kMutableCmdline)) {}

void Configuration::UpdateFromCommandLine(int argc, const char* const* argv,
                                          CommandLineOptions options,
                                          std::string* err_description) {}

void Configuration::UpdateFromString(const std::string& json_str,
                                     std::string* err_description) {}

void Configuration::UpdateFromFile(const std::string& filename,
                                   std::string* err_description) {}

std::string Configuration::Dump(bool resolve_variables) const { return {}; }

void Configuration::WriteToFile(const std::string& filename,
                                bool resolve_variables,
                                int identation_width) const {}

const LoggerPtr Configuration::logger_ =
    Logger::CreateStaticInternalLogger("Configuration");

}  // namespace objects
