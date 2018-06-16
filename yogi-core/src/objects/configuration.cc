#include "configuration.h"

#include <boost/algorithm/string.hpp>
#include <string>
#include <sstream>
#include <fstream>
using namespace std::string_literals;

namespace objects {

Configuration::Configuration(ConfigurationFlags flags)
    : variables_supported_(!(flags & kDisableVariables)),
      mutable_cmdline_(flags & kMutableCmdline),
      json_({}),
      immutable_json_({}) {}

void Configuration::UpdateFromCommandLine(int argc, const char* const* argv,
                                          CommandLineOptions options,
                                          std::string* err_desc) {
  detail::CommandLineParser parser(argc, argv, options);

  try {
    parser.Parse();
  } catch (const api::Error& err) {
    *err_desc = parser.GetLastErrorString();
    throw;
  }

  VerifyAndMerge(parser.GetFilesConfiguration(),
                 parser.GetDirectConfiguration(), err_desc);

  if (!mutable_cmdline_) {
    immutable_json_ = parser.GetDirectConfiguration();
  }
}

void Configuration::UpdateFromString(const std::string& json_str,
                                     std::string* err_desc) {
  nlohmann::json json;

  try {
    json = nlohmann::json::parse(json_str);
  } catch (const nlohmann::json::exception& e) {
    *err_desc = "Could not parse JSON string: "s + e.what();
    throw api::Error(YOGI_ERR_PARSING_JSON_FAILED);
  }

  VerifyAndMerge(json, immutable_json_, err_desc);
}

void Configuration::UpdateFromFile(const std::string& filename,
                                   std::string* err_desc) {
  std::ifstream f(filename);
  if (!f.is_open()) {
    *err_desc = "Could not open "s + filename;
    throw api::Error(YOGI_ERR_PARSING_FILE_FAILED);
  }

  nlohmann::json json;
  try {
    f >> json;
  } catch (const std::exception& e) {
    *err_desc = "Could not parse "s + filename + ": " + e.what();
    throw api::Error(YOGI_ERR_PARSING_FILE_FAILED);
  }

  VerifyAndMerge(json, immutable_json_, err_desc);
}

std::string Configuration::Dump(bool resolve_variables,
                                int indentation_width) const {
  if (resolve_variables) {
    if (!variables_supported_) {
      throw api::Error(YOGI_ERR_NO_VARIABLE_SUPPORT);
    }

    return ResolveVariables(json_, nullptr).dump(indentation_width);
  } else {
    return json_.dump(indentation_width);
  }
}

void Configuration::WriteToFile(const std::string& filename,
                                bool resolve_variables,
                                int indentation_width) const {
  if (!variables_supported_ && resolve_variables) {
    throw api::Error(YOGI_ERR_NO_VARIABLE_SUPPORT);
  }

  std::ofstream f(filename);
  if (!f.is_open()) {
    throw api::Error(YOGI_ERR_OPEN_FILE_FAILED);
  }

  try {
    if (resolve_variables) {
      f << ResolveVariables(json_, nullptr).dump(indentation_width);
    } else {
      f << json_.dump(indentation_width);
    }

    if (indentation_width != -1) {
      f << std::endl;
    }
  } catch (const std::exception& e) {
    YOGI_LOG_ERROR(logger_, "Could not write configuration to "
                                << filename << ": " << e.what());
    throw api::Error(YOGI_ERR_WRITE_TO_FILE_FAILED);
  }
}

void Configuration::CheckCircularVariableDependency(
    const std::string& var_ref, const nlohmann::json& var_val,
    std::string* err_desc) {
  if (var_val.is_string()) {
    auto str = var_val.get<std::string>();
    if (str.find(var_ref) != std::string::npos) {
      *err_desc = "Circular dependency in "s + var_ref;
      throw api::Error(YOGI_ERR_UNDEFINED_VARIABLES);
    }
  }
}

void Configuration::ResolveVariablesSections(nlohmann::json* vars,
                                             std::string* err_desc) {
  for (auto it = vars->begin(); it != vars->end(); ++it) {
    auto var_ref = "${"s + it.key() + '}';
    auto var_val = it.value();
    CheckCircularVariableDependency(var_ref, var_val, err_desc);

    for (auto& elem : *vars) {
      ResolveSingleVariable(&elem, var_ref, var_val);
    }
  }
}

void Configuration::ResolveSingleVariable(nlohmann::json* elem,
                                          const std::string& var_ref,
                                          const nlohmann::json& var_val) {
  if (!elem->is_string()) {
    return;
  }

  auto val = elem->get<std::string>();
  if (val == var_ref) {
    *elem = var_val;
  } else {
    boost::replace_all(val, var_ref, var_val.dump());
    *elem = val;
  }
}

nlohmann::json Configuration::ResolveVariables(
    const nlohmann::json& unresolved_json, std::string* err_desc) {
  auto json = unresolved_json;
  if (!json.count("variables")) {
    return json;
  }

  auto& vars = json["variables"];

  ResolveVariablesSections(&vars, err_desc);

  WalkAllElements(&json, [&](const auto& key, auto* elem) {
    for (auto it = vars.cbegin(); it != vars.cend(); ++it) {
      auto var_ref = "${"s + it.key() + '}';
      ResolveSingleVariable(elem, var_ref, it.value());
    }
  });

  return json;
}

template <typename Fn>
void Configuration::WalkAllElements(nlohmann::json* json, Fn fn) {
  for (auto it = json->begin(); it != json->end(); ++it) {
    if (it.value().is_structured()) {
      WalkAllElements(&it.value(), fn);
    }

    fn(it.key(), &it.value());
  }
}

void Configuration::CheckVariablesOnlyUsedInValues(nlohmann::json* json,
                                                   std::string* err_desc) {
  WalkAllElements(json, [=](const auto& key, auto) {
    if (key.find("${") != std::string::npos) {
      *err_desc = "Found syntax for variable in key: "s + key;
      throw api::Error(YOGI_ERR_VARIABLE_USED_IN_KEY);
    }
  });
}

void Configuration::VerifyAndMerge(const nlohmann::json& json_to_merge,
                                   const nlohmann::json& immutable_json,
                                   std::string* err_desc) {
  auto new_json = json_;
  new_json.merge_patch(json_to_merge);
  new_json.merge_patch(immutable_json);

  if (variables_supported_) {
    CheckVariablesOnlyUsedInValues(&new_json, err_desc);
    ResolveVariables(new_json, err_desc);
  }

  json_ = new_json;
  err_desc->clear();
}

const LoggerPtr Configuration::logger_ =
    Logger::CreateStaticInternalLogger("Configuration");

}  // namespace objects
