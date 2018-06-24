#include "macros.h"
#include "helpers.h"
#include "../api/constants.h"
#include "../objects/branch.h"
#include "../utils/system.h"

YOGI_API int YOGI_BranchCreate(void** branch, void* context, const char* name,
                               const char* description, const char* netname,
                               const char* password, const char* path,
                               const char* advaddr, int advport,
                               long long advint, long long timeout) {
  CHECK_PARAM(branch != nullptr);
  CHECK_PARAM(context != nullptr);
  CHECK_PARAM(name == nullptr || *name != '\0');
  CHECK_PARAM(netname == nullptr || *netname != '\0');
  CHECK_PARAM(password == nullptr || *password != '\0');
  CHECK_PARAM(path == nullptr || *path == '/');
  CHECK_PARAM(advaddr == nullptr || *advaddr != '\0');
  CHECK_PARAM(advport >= 0);
  CHECK_PARAM(advint == -1 || advint == 0 || advint >= 1000000);
  CHECK_PARAM(timeout == -1 || timeout == 0 || timeout >= 1000000);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    auto final_name = name ? std::string(name)
                           : std::to_string(utils::GetProcessId()) + '@' +
                                 utils::GetHostname();

    boost::system::error_code ec;
    auto adv_ep = boost::asio::ip::udp::endpoint(
        boost::asio::ip::make_address(
            advaddr ? advaddr : api::kDefaultAdvAddress, ec),
        static_cast<unsigned short>(advport ? advport : api::kDefaultAdvPort));
    if (ec) return YOGI_ERR_INVALID_PARAM;

    auto brn = objects::Branch::Create(
        ctx, final_name, description ? description : "",
        netname ? std::string(netname) : utils::GetHostname(),
        password ? password : "",
        path ? std::string(path) : (std::string("/") + final_name), adv_ep,
        advint ? ConvertDuration(advint)
               : std::chrono::nanoseconds(api::kDefaultAdvInterval),
        timeout ? ConvertDuration(timeout)
                : std::chrono::nanoseconds(api::kDefaultConnectionTimeout));
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
