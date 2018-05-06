#pragma once

#include "../../../config.h"
#include "../../../utils/socket.h"
#include "../../context.h"
#include "../../logger.h"
#include "branch_info.h"

#include <memory>
#include <functional>

namespace objects {
namespace detail {

class InfoQuerier : public std::enable_shared_from_this<InfoQuerier> {
 public:
  InfoQuerier(std::chrono::nanoseconds timeout,
              std::chrono::nanoseconds retry_time);

  void QueryBranch(const BranchInfoPtr& info,
                   std::function<void(const utils::TimedTcpSocketPtr& socket)>&&
                       success_handler);

 private:
  static const LoggerPtr logger_;

  const std::chrono::nanoseconds timeout_;
  const std::chrono::nanoseconds retry_time_;
};

typedef std::shared_ptr<InfoQuerier> InfoQuerierPtr;

}  // namespace detail
}  // namespace objects
