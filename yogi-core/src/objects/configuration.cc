#include "configuration.h"

#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <sstream>
using namespace std::string_literals;

namespace objects {

Configuration::Configuration(ConfigurationFlags flags)
    : variables_supported_(!(flags & kDisableVariables)),
      mutable_cmdline_(!(flags & kMutableCmdline)) {}

void Configuration::UpdateFromCommandLine(int argc, const char* const* argv,
                                          CommandLineOptions options,
                                          std::string* err_description) {
  namespace po = boost::program_options;

  po::options_description visible_options("Allowed options");
  visible_options.add_options()("help,h", "Show this help message");
  po::options_description hidden_options("Hidden options");
  po::positional_options_description positional_options;

  // clang-format off
  boost::optional<bool> log_colour;
  boost::optional<std::string> log_file;
  boost::optional<std::string> log_console;
  boost::optional<std::string> log_fmt;
  boost::optional<std::string> log_time_fmt;
  std::vector<std::string> log_verbosities;
  if (options & kLoggingOptions) {
    visible_options.add_options()(
      "help-logging",
      "Shows information about the logging options"
    )(
      "log-file", po::value(&log_file),
      "Path to the logfile (supports time placeholders)"
    )(
      "log-console", po::value(&log_console)->implicit_value("STDERR"s),
      "Log to either STDOUT or STDERR (default is STDERR)"
    )(
      "log-colour,logcolor", po::value(&log_colour)->implicit_value(true),
      "Use colour when logging to the console"
    )(
      "log-fmt", po::value(&log_fmt),
      "Format of a log entry (supports entry placeholders)"
    )(
      "log-timefmt", po::value(&log_time_fmt),
      "Format of a log entry's timestamp (supports time placeholders)"
    )(
      "log-verbosity", po::value(&log_verbosities)->composing(),
      "Configuration variables (e.g. --log-verbosity='Yogi.*:DEBUG')"
    );
  }

  boost::optional<std::string> branch_name;
  if (options & kBranchNameOption) {
    visible_options.add_options()(
      "name", po::value(&branch_name),
      "Branch name"
    );
  }

  boost::optional<std::string> branch_description;
  if (options & kBranchDescriptionOption) {
    visible_options.add_options()(
      "description", po::value(&branch_description),
      "Branch description"
    );
  }

  boost::optional<std::string> branch_network;
  if (options & kBranchNetworkOption) {
    visible_options.add_options()(
      "network", po::value(&branch_network),
      "Network name"
    );
  }

  boost::optional<std::string> branch_password;
  if (options & kBranchPasswordOption) {
    visible_options.add_options()(
      "password", po::value(&branch_password),
      "Network password"
    );
  }

  boost::optional<std::string> branch_path;
  if (options & kBranchPathOption) {
    visible_options.add_options()(
      "path", po::value(&branch_path),
      "Branch path"
    );
  }

  boost::optional<std::string> branch_adv_addr;
  if (options & kBranchAdvaddrOption) {
    visible_options.add_options()(
      "adv-addr", po::value(&branch_adv_addr),
      "Branch advertising address (e.g. --adv-addr='ff31::8000:2439')"
    );
  }

  boost::optional<int> branch_adv_port;
  if (options & kBranchAdvportOption) {
    visible_options.add_options()(
      "adv-port", po::value(&branch_adv_port),
      "Branch advertising port (e.g. --adv-port=13531"
    );
  }

  boost::optional<float> branch_adv_interval;
  if (options & kBranchAdvintOption) {
    visible_options.add_options()(
      "adv-int", po::value(&branch_adv_interval),
      "Branch advertising interval in seconds (e.g. --adv-int=3.0)"
    );
  }

  boost::optional<float> branch_timeout;
  if (options & kBranchTimeoutOption) {
    visible_options.add_options()(
      "timeout", po::value(&branch_timeout),
      "Branch timeout in seconds (e.g. --timeout=3.0)"
    );
  }

  std::vector<std::string> cfg_file_patterns;
  if (options & kFileOption || options & kFileRequiredOption) {
    auto name = "_cfg_files";
    hidden_options.add_options()(
      name, po::value(&cfg_file_patterns),
      "Configuration files (JSON format)"
    );

    positional_options.add(name, -1);
  }

  std::vector<std::string> overrides;
  if (options & kOverrideOption) {
    visible_options.add_options()(
      "override,o", po::value(&overrides)->composing(),
      "Configuration overrides in simplified (--override='person.age:42') or"
        " JSON (--override='{\"person\":{\"age\":42}}') format"
    );
  }

  std::vector<std::string> variables;
  if (options & kVariableOption) {
    visible_options.add_options()(
      "var,v", po::value(&variables)->composing(),
      "Configuration variables (e.g. --var='DIR=\"/usr/local\"'"
    );
  }
  // clang-format on

  po::variables_map vm;
  po::options_description all_options;
  all_options.add(visible_options);
  all_options.add(hidden_options);

  try {
    po::store(po::command_line_parser(argc, argv)
                  .options(all_options)
                  .positional(positional_options)
                  .run(),
              vm);
    po::notify(vm);
  } catch (const po::error& e) {
    *err_description = e.what();
    throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
  }

  if (vm.count("help")) {
    std::string binary_name = argv[0];
    auto pos = binary_name.find_last_of("/\\");
    if (pos != std::string::npos) {
      binary_name = binary_name.substr(pos + 1);
    }

    std::stringstream ss;
    ss << "Usage: " << binary_name << " [options]";
    if (options & kFileOption || options & kFileRequiredOption) {
      ss << (options & kFileRequiredOption ? " config.json" : " [config.json]");
      ss << " [config2.json ...]";
    }
    ss << std::endl;

    ss << std::endl;
    ss << visible_options << std::endl;

    *err_description = ss.str();
    throw api::Error(YOGI_ERR_HELP_REQUESTED);
  }

  if (vm.count("help-logging")) {
    // clang-format off
    std::stringstream ss;
    ss << "Some of the logging switches support the placeholders listed below." << std::endl;
    ss << std::endl;
    ss << "Time placeholders:" << std::endl;
    ss << "  %Y - Four digit year" << std::endl;
    ss << "  %m - Month name as a decimal 01 to 12" << std::endl;
    ss << "  %d - Day of the month as decimal 01 to 31" << std::endl;
    ss << "  %F - Equivalent to %Y-%m-%d (the ISO 8601 date format)" << std::endl;
    ss << "  %H - The hour as a decimal number using a 24-hour clock (range 00 to 23)" << std::endl;
    ss << "  %M - The minute as a decimal 00 to 59" << std::endl;
    ss << "  %S - Seconds as a decimal 00 to 59" << std::endl;
    ss << "  %T - Equivalent to %H:%M:%S (the ISO 8601 time format)" << std::endl;
    ss << "  %3 - Milliseconds as decimal number 000 to 999" << std::endl;
    ss << "  %6 - Microseconds as decimal number 000 to 999" << std::endl;
    ss << "  %9 - Nanoseconds as decimal number 000 to 999" << std::endl;
    ss << std::endl;
    ss << "Entry placeholders:" << std::endl;
    ss << "  $t - Timestamp, formatted according to \p timefmt" << std::endl;
    ss << "  $P - Process ID (PID)" << std::endl;
    ss << "  $T - Thread ID" << std::endl;
    ss << "  $s - Severity as a 3 letter abbreviation (FAT, ERR, WRN, IFO, DBG or TRC)" << std::endl;
    ss << "  $m - Log message" << std::endl;
    ss << "  $f - Source filename" << std::endl;
    ss << "  $l - Source line number" << std::endl;
    ss << "  $c - Component tag" << std::endl;
    ss << "  $< - Set console colour corresponding to severity" << std::endl;
    ss << "  $> - Reset the colours (also done after each log entry)" << std::endl;
    ss << "  $$ - A $ sign" << std::endl;
    // clang-format on
  }

  if (log_console) {
    boost::algorithm::to_upper(*log_console);
    if (*log_console != "STDERR" && *log_console != "STDOUT") {
      *err_description =
          "Invalid value \""s + *log_console +
          "\"for --log-console. Allowed values are STDOUT and STDERR.";
      throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
    }
  }

  if (options & kFileRequiredOption && cfg_file_patterns.empty()) {
    *err_description = "No configuration files specified.";
    throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
  }
}

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
