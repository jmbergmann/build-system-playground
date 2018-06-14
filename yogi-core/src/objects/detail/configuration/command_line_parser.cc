#include "command_line_parser.h"
#include "../../../api/error.h"

#include <boost/algorithm/string.hpp>
#include <boost/optional/optional_io.hpp>
#include <sstream>

namespace po = boost::program_options;
using namespace std::string_literals;

namespace objects {
namespace detail {

CommandLineParser::CommandLineParser(int argc, const char* const* argv,
                                     CommandLineOptions options)
    : argc_(argc),
      argv_(argv),
      options_(options),
      visible_options_("Allowed options"),
      hidden_options_("Hidden options") {}

nlohmann::json CommandLineParser::Parse() {
  YOGI_ASSERT(vm_.empty());  // only run once!

  AddHelpOptions();
  AddLoggingOptions();
  AddBranchOptions();
  AddFilesOption();
  AddOverrideOption();
  AddVariableOption();

  PopulateVariablesMap();

  VerifyHelpOption();
  VerifyHelpLoggingOption();
  VerifyLogConsoleOption();
  VerifyFilesOption();

  return {};
}

void CommandLineParser::AddHelpOptions() {
  // clang-format off
  visible_options_.add_options()(
    "help,h",
    "Show this help message"
  );

  if (options_ & kLoggingOptions) {
    visible_options_.add_options()(
      "help-logging",
      "Shows information about the logging options"
    );
  }
  // clang-format on
}

void CommandLineParser::AddLoggingOptions() {
  // clang-format off
  if (options_ & kLoggingOptions) {
    visible_options_.add_options()(
      "log-file", po::value(&log_file_),
      "Path to the logfile (supports time placeholders)"
    )(
      "log-console", po::value(&log_console_)->notifier([&](auto& val) {
        log_console_ = boost::algorithm::to_upper_copy(*val);
      })->implicit_value("STDERR"s),
      "Log to either STDOUT or STDERR (default is STDERR)"
    )(
      "log-colour,logcolor", po::value(&log_colour_)->implicit_value(true),
      "Use colour when logging to the console"
    )(
      "log-fmt", po::value(&log_fmt_),
      "Format of a log entry (supports entry placeholders)"
    )(
      "log-timefmt", po::value(&log_time_fmt_),
      "Format of a log entry's timestamp (supports time placeholders)"
    )(
      "log-verbosity", po::value(&log_verbosities_)->composing(),
      "Configuration variables (e.g. --log-verbosity='Yogi.*:DEBUG')"
    );
  }
  // clang-format on
}

void CommandLineParser::AddBranchOptions() {
  // clang-format off
  if (options_ & kBranchNameOption) {
    visible_options_.add_options()(
      "name", po::value(&branch_name_),
      "Branch name"
    );
  }

  if (options_ & kBranchDescriptionOption) {
    visible_options_.add_options()(
      "description", po::value(&branch_description_),
      "Branch description"
    );
  }

  if (options_ & kBranchNetworkOption) {
    visible_options_.add_options()(
      "network", po::value(&branch_network_),
      "Network name"
    );
  }

  if (options_ & kBranchPasswordOption) {
    visible_options_.add_options()(
      "password", po::value(&branch_password_),
      "Network password"
    );
  }

  if (options_ & kBranchPathOption) {
    visible_options_.add_options()(
      "path", po::value(&branch_path_),
      "Branch path"
    );
  }

  if (options_ & kBranchAdvaddrOption) {
    visible_options_.add_options()(
      "adv-addr", po::value(&branch_adv_addr_),
      "Branch advertising address (e.g. --adv-addr='ff31::8000:2439')"
    );
  }

  if (options_ & kBranchAdvportOption) {
    visible_options_.add_options()(
      "adv-port", po::value(&branch_adv_port_),
      "Branch advertising port (e.g. --adv-port=13531"
    );
  }

  if (options_ & kBranchAdvintOption) {
    visible_options_.add_options()(
      "adv-int", po::value(&branch_adv_interval_),
      "Branch advertising interval in seconds (e.g. --adv-int=3.0)"
    );
  }

  if (options_ & kBranchTimeoutOption) {
    visible_options_.add_options()(
      "timeout", po::value(&branch_timeout_),
      "Branch timeout in seconds (e.g. --timeout=3.0)"
    );
  }
  // clang-format on
}

void CommandLineParser::AddFilesOption() {
  // clang-format off
  if (options_ & kFileOption || options_ & kFileRequiredOption) {
    auto name = "_cfg_files";
    hidden_options_.add_options()(
      name, po::value(&cfg_file_patterns_),
      "Configuration files (JSON format)"
    );

    positional_options_.add(name, -1);
  }
  // clang-format on
}

void CommandLineParser::AddOverrideOption() {
  // clang-format off
  if (options_ & kOverrideOption) {
    visible_options_.add_options()(
      "override,o", po::value(&overrides_)->composing(),
      "Configuration overrides in simplified (--override='person.age:42') or"
        " JSON (--override='{\"person\":{\"age\":42}}') format"
    );
  }
  // clang-format on
}

void CommandLineParser::AddVariableOption() {
  // clang-format off
  if (options_ & kVariableOption) {
    visible_options_.add_options()(
      "var,v", po::value(&variables_)->composing(),
      "Configuration variables (e.g. --var='DIR=\"/usr/local\"'"
    );
  }
  // clang-format on
}

void CommandLineParser::PopulateVariablesMap() {
  po::options_description options;
  options.add(visible_options_);
  options.add(hidden_options_);

  try {
    po::store(po::command_line_parser(argc_, argv_)
                  .options(options)
                  .positional(positional_options_)
                  .run(),
              vm_);
    po::notify(vm_);
  } catch (const po::error& e) {
    err_description_ = e.what();
    throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
  }
}

void CommandLineParser::VerifyHelpOption() {
  if (vm_.count("help")) {
    std::string binary_name = argv_[0];
    auto pos = binary_name.find_last_of("/\\");
    if (pos != std::string::npos) {
      binary_name = binary_name.substr(pos + 1);
    }

    std::stringstream ss;
    ss << "Usage: " << binary_name << " [options]";
    if (options_ & kFileOption || options_ & kFileRequiredOption) {
      ss << (options_ & kFileRequiredOption ? " config.json"
                                            : " [config.json]");
      ss << " [config2.json ...]";
    }
    ss << std::endl;

    ss << std::endl;
    ss << visible_options_ << std::endl;

    err_description_ = ss.str();
    throw api::Error(YOGI_ERR_HELP_REQUESTED);
  }
}

void CommandLineParser::VerifyHelpLoggingOption() {
  if (vm_.count("help-logging")) {
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
    ss << "  $t - Timestamp, formatted according to the configured time format" << std::endl;
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

    err_description_ = ss.str();
    throw api::Error(YOGI_ERR_HELP_REQUESTED);
  }
}

void CommandLineParser::VerifyLogConsoleOption() {
  if (!log_console_) {
    return;
  }

  if (*log_console_ != "STDERR" && *log_console_ != "STDOUT") {
    err_description_ =
        "Invalid value \""s + *log_console_ +
        "\"for --log-console. Allowed values are STDOUT and STDERR.";
    throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
  }
}

void CommandLineParser::VerifyFilesOption() {
  if (options_ & kFileRequiredOption && cfg_file_patterns_.empty()) {
    err_description_ = "No configuration files specified.";
    throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
  }
}

}  // namespace detail
}  // namespace objects
