#include "macros.h"
#include "helpers.h"
#include "../api/constants.h"
#include "../objects/branch.h"
#include "../utils/system.h"
#include "../../../3rd_party/nlohmann/json.hpp"

#include <string>
using namespace std::string_literals;

YOGI_API int YOGI_BranchCreate(void** branch, void* context, const char* props,
                               const char* section, char* err, int errsize) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(context != nullptr);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    std::string err_desc;

    nlohmann::json properties;
    if (props) {
      try {
        properties = nlohmann::json::parse(props);
      } catch (const nlohmann::json::exception& e) {
        err_desc = "Could not parse JSON string: "s + e.what();
        CopyStringToUserBuffer(err_desc, err, errsize);
        throw api::Error(YOGI_ERR_PARSING_JSON_FAILED);
      }

      if (section) {
        if (!properties[section].is_object()) {
          err_desc = "Could not find section \""s + section +
                     "\" in branch properties.";
          CopyStringToUserBuffer(err_desc, err, errsize);
          throw api::Error(YOGI_ERR_PARSING_JSON_FAILED);
        }

        properties = properties[section];
      }
    }

    CopyStringToUserBuffer(err_desc, err, errsize);

    auto name = properties.value("name", std::to_string(utils::GetProcessId()) +
                                             '@' + utils::GetHostname());
    auto adv_addr = properties.value<std::string>("advertising_address",
                                                  api::kDefaultAdvAddress);
    auto adv_port = properties.value("advertising_port", api::kDefaultAdvPort);
    auto adv_int = ExtractDuration(properties, "advertising_interval",
                                   api::kDefaultAdvInterval);
    auto description = properties.value("description", std::string{});
    auto network = properties.value("network_name", utils::GetHostname());
    auto password = properties.value("network_password", std::string{});
    auto path = properties.value("path", "/"s + name);

    if (adv_addr.empty()) return YOGI_ERR_INVALID_PARAM;
    boost::system::error_code ec;
    auto adv_ep = boost::asio::ip::udp::endpoint(
        boost::asio::ip::make_address(adv_addr, ec),
        static_cast<unsigned short>(adv_port));
    if (ec) return YOGI_ERR_INVALID_PARAM;

    auto timeout =
        ExtractDuration(properties, "timeout", api::kDefaultConnectionTimeout);

    auto brn =
        objects::Branch::Create(ctx, name, description, network, password, path,
                                adv_ep, adv_int, timeout);
    brn->Start();

    *branch = api::ObjectRegister::Register(brn);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchGetInfo(void* branch, void* uuid, char* json,
                                int jsonsize) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(json == nullptr || jsonsize > 0);

  try {
    auto brn = api::ObjectRegister::Get<objects::Branch>(branch);
    CopyUuidToUserBuffer(brn->GetUuid(), uuid);

    if (!CopyStringToUserBuffer(brn->MakeInfoString(), json, jsonsize)) {
      return YOGI_ERR_BUFFER_TOO_SMALL;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchGetConnectedBranches(void* branch, void* uuid,
                                             char* json, int jsonsize,
                                             void (*fn)(int, void*),
                                             void* userarg) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(json == nullptr || jsonsize > 0);
  CHECK_PARAM(fn != nullptr);

  try {
    auto brn = api::ObjectRegister::Get<objects::Branch>(branch);

    auto buffer_too_small = false;
    for (auto& entry : brn->MakeConnectedBranchesInfoStrings()) {
      CopyUuidToUserBuffer(entry.first, uuid);

      if (CopyStringToUserBuffer(entry.second, json, jsonsize)) {
        fn(YOGI_OK, userarg);
      } else {
        fn(YOGI_ERR_BUFFER_TOO_SMALL, userarg);
        buffer_too_small = true;
      }
    }

    if (buffer_too_small) {
      return YOGI_ERR_BUFFER_TOO_SMALL;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchAwaitEvent(void* branch, int events, void* uuid,
                                   char* json, int jsonsize,
                                   void (*fn)(int, int, int, void*),
                                   void* userarg) {
  using BranchEvents = objects::Branch::BranchEvents;

  CHECK_PARAM(branch != nullptr);
  CHECK_FLAGS(events, BranchEvents::kAllEvents);
  CHECK_PARAM(json == nullptr || jsonsize > 0);
  CHECK_PARAM(fn != nullptr);

  try {
    auto brn = api::ObjectRegister::Get<objects::Branch>(branch);

    brn->AwaitEvent(
        ConvertFlags(events, BranchEvents::kNoEvent),
        [=](auto& res, auto event, auto& evres, auto& tmp_uuid,
            auto& tmp_json) {
          if (res != api::kSuccess) {
            fn(res.error_code(), event, evres.error_code(), userarg);
            return;
          }

          CopyUuidToUserBuffer(tmp_uuid, uuid);
          if (CopyStringToUserBuffer(tmp_json, json, jsonsize)) {
            fn(res.error_code(), event, evres.error_code(), userarg);
          } else {
            fn(YOGI_ERR_BUFFER_TOO_SMALL, event, evres.error_code(), userarg);
          }
        });
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_BranchCancelAwaitEvent(void* branch) {
  CHECK_PARAM(branch != nullptr);

  try {
    auto brn = api::ObjectRegister::Get<objects::Branch>(branch);
    brn->CancelAwaitEvent();
  }
  CATCH_AND_RETURN;
}
