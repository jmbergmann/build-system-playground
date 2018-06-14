#include "command_line_parser.h"
#include "../../api/error.h"
#include "../../objects/logger.h"
#include "../../utils/glob.h"

#include <boost/algorithm/string.hpp>
#include <boost/optional/optional_io.hpp>
#include <sstream>
#include <algorithm>

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
  AddFileOptions();
  AddOverrideOptions();
  AddVariableOptions();

  PopulateVariablesMap();
  HandleHelpOptions();
  ExtractOptions();

  return json_;
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
      "log-file", po::value<std::string>()->notifier([&](auto& val) {
        LogFileNotifier(val);
      }),
      "Path to the logfile with support for time placeholders; set to NONE to disable"
    )(
      "log-console", po::value<std::string>()->notifier([&](auto& val) {
        LogConsoleNotifier(val);
      })->implicit_value("STDERR"s),
      "Log to either STDOUT, STDERR or NONE (implicit value is STDERR)"
    )(
      "log-colour", po::value<bool>()->notifier([&](auto& val) {
        json_["logging"]["colour"] = val;
      })->implicit_value(true),
      "Use colour when logging to the console"
    )(
      "log-fmt", po::value<std::string>()->notifier([&](auto& val) {
        json_["logging"]["entry-format"] = val;
      }),
      "Format of a log entry (use entry placeholders)"
    )(
      "log-timefmt", po::value<std::string>()->notifier([&](auto& val) {
        json_["logging"]["time-format"] = val;
      }),
      "Format of a log entry's timestamp (use time placeholders)"
    )(
      "log-verbosity", po::value<std::vector<std::string>>()->notifier([&](auto& val) {
        LogVerbosityNotifier(val);
      }),
      "Configuration variables (e.g. --log-verbosity='Yogi.*=DEBUG')"
    );
  }
  // clang-format on
}

void CommandLineParser::AddBranchOptions() {
  // clang-format off
  if (options_ & kBranchNameOption) {
    visible_options_.add_options()(
      "name", po::value<std::string>()->notifier([&](auto& val) {
        json_["branch"]["name"] = val;
      }),
      "Branch name"
    );
  }

  if (options_ & kBranchDescriptionOption) {
    visible_options_.add_options()(
      "description", po::value<std::string>()->notifier([&](auto& val) {
        json_["branch"]["description"] = val;
      }),
      "Branch description"
    );
  }

  if (options_ & kBranchNetworkOption) {
    visible_options_.add_options()(
      "network", po::value<std::string>()->notifier([&](auto& val) {
        json_["branch"]["network"] = val;
      }),
      "Network name"
    );
  }

  if (options_ & kBranchPasswordOption) {
    visible_options_.add_options()(
      "password", po::value<std::string>()->notifier([&](auto& val) {
        json_["branch"]["password"] = val;
      }),
      "Network password"
    );
  }

  if (options_ & kBranchPathOption) {
    visible_options_.add_options()(
      "path", po::value<std::string>()->notifier([&](auto& val) {
        json_["branch"]["path"] = val;
      }),
      "Branch path"
    );
  }

  if (options_ & kBranchAdvaddrOption) {
    visible_options_.add_options()(
      "adv-addr", po::value<std::string>()->notifier([&](auto& val) {
        json_["branch"]["advertising-address"] = val;
      }),
      "Branch advertising address (e.g. --adv-addr 'ff31::8000:2439')"
    );
  }

  if (options_ & kBranchAdvportOption) {
    visible_options_.add_options()(
      "adv-port", po::value<unsigned>()->notifier([&](auto& val) {
        json_["branch"]["advertising-port"] = val;
      }),
      "Branch advertising port (e.g. --adv-port 13531"
    );
  }

  if (options_ & kBranchAdvintOption) {
    visible_options_.add_options()(
      "adv-int", po::value<float>()->notifier([&](auto& val) {
        json_["branch"]["advertising-interval"] = val;
      }),
      "Branch advertising interval in seconds (e.g. --adv-int 3.0)"
    );
  }

  if (options_ & kBranchTimeoutOption) {
    visible_options_.add_options()(
      "timeout", po::value<float>()->notifier([&](auto& val) {
        json_["branch"]["timeout"] = val;
      }),
      "Branch timeout in seconds (e.g. --timeout 3.0)"
    );
  }
  // clang-format on
}

void CommandLineParser::AddFileOptions() {
  // clang-format off
  if (options_ & kFileOption || options_ & kFileRequiredOption) {
    auto name = "_cfg_files";
    hidden_options_.add_options()(
      name, po::value<std::vector<std::string>>()->notifier([&](auto& val) {
        FileNotifier(val);
      }),
      "Configuration files (JSON format)"
    );

    positional_options_.add(name, -1);
  }
  // clang-format on
}

void CommandLineParser::AddOverrideOptions() {
  // clang-format off
  if (options_ & kOverrideOption) {
    visible_options_.add_options()(
      "override,o", po::value<std::vector<std::string>>()->notifier([&](auto& val) {
        OverrideNotifier(val);
      }),
      "Configuration overrides in simplified (--override 'person.age=42') or"
        " JSON (--override '{\"person\":{\"age\":42}}') format"
    );
  }
  // clang-format on
}

void CommandLineParser::AddVariableOptions() {
  // clang-format off
  if (options_ & kVariableOption) {
    visible_options_.add_options()(
      "var,v", po::value<std::vector<std::string>>()->notifier([&](auto& val) {
        VariableNotifier(val);
      }),
      "Configuration variables (e.g. --var 'DIR=\"/usr/local\"')"
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
  } catch (const po::error& e) {
    err_description_ = e.what();
    throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
  }
}

void CommandLineParser::HandleHelpOptions() {
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

void CommandLineParser::ExtractOptions() {
  try {
    po::notify(vm_);
  } catch (const po::error& e) {
    err_description_ = e.what();
    throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
  }
}

void CommandLineParser::LogFileNotifier(const std::string& val) {
  if (boost::to_upper_copy(val) == "NONE") {
    json_["logging"]["file"] = nullptr;
  } else {
    json_["logging"]["file"] = val;
  }
}

void CommandLineParser::LogConsoleNotifier(const std::string& val) {
  auto s = boost::to_upper_copy(val);
  if (s != "STDERR" && s != "STDOUT" && s != "NONE") {
    err_description_ = "Invalid value \""s + val +
                       "\"for --log-console."
                       " Allowed values are STDOUT, STDERR and NONE.";
    throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
  }

  if (s == "NONE") {
    json_["logging"]["console"] = nullptr;
  } else {
    json_["logging"]["console"] = s;
  }
}

void CommandLineParser::LogVerbosityNotifier(
    const std::vector<std::string>& val) {
  for (auto& str : val) {
    auto sep_pos = str.find('=');
    if (sep_pos == std::string::npos) {
      err_description_ = "Invalid log verbosity string format \""s + str + "\"";
      throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
    }

    auto comp = str.substr(0, sep_pos);
    auto verb = str.substr(sep_pos + 1);

    try {
      Logger::StringToVerbosity(verb);
    } catch (const std::exception& e) {
      err_description_ = e.what();
      throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
    }

    json_["verbosity"][comp] = verb;
  }
}

void CommandLineParser::FileNotifier(const std::vector<std::string>& val) {
  try {
    config_files_ = utils::Glob(val);
  } catch (const std::exception& e) {
    err_description_ = e.what();
    throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
  }

  if (options_ & kFileRequiredOption && config_files_.empty()) {
    err_description_ = "No configuration files specified.";
    throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
  }
}

void CommandLineParser::OverrideNotifier(const std::vector<std::string>& val) {
  for (auto& str : val) {
    std::string json_str;
    if (str.front() == '{') {
      json_str = str;
    } else {
      auto sep_pos = str.find('=');
      if (sep_pos == std::string::npos) {
        err_description_ = "Invalid override format \""s + str + "\"";
        throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
      }

      auto path = str.substr(0, sep_pos);
      auto value = str.substr(sep_pos + 1);

      std::vector<std::string> path_elements;
      boost::algorithm::split(path_elements, path, boost::is_any_of("."));

      std::stringstream ss;
      for (auto& elem : path_elements) {
        ss << "{\"" << elem << "\":";
      }
      ss << value;
      for (auto it = path_elements.begin(); it != path_elements.end(); ++it) {
        ss << "}";
      }

      json_str = ss.str();
    }

    try {
      overrides_.emplace_back(nlohmann::json::parse(json_str));
    } catch (const std::exception& e) {
      err_description_ = "Parsing \""s + str + "\" failed: " + e.what();
      throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
    }
  }
}

void CommandLineParser::VariableNotifier(const std::vector<std::string>& val) {
  for (auto& str : val) {
    auto sep_pos = str.find('=');
    if (sep_pos == std::string::npos) {
      err_description_ = "Invalid veriable format \""s + str + "\"";
      throw api::Error(YOGI_ERR_PARSING_CMDLINE_FAILED);
    }

    auto name = str.substr(0, sep_pos);
    auto value = str.substr(sep_pos + 1);

    json_["variables"][name] = value;
  }
}

}  // namespace detail
}  // namespace objects
