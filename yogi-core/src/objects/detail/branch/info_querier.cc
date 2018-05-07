#include "info_querier.h"

namespace objects {
namespace detail {

const LoggerPtr InfoQuerier::logger_ =
    Logger::CreateStaticInternalLogger("Branch");

InfoQuerier::InfoQuerier(std::chrono::nanoseconds timeout,
                         std::chrono::nanoseconds retry_time)
    : timeout_(timeout), retry_time_(retry_time) {}

void InfoQuerier::QueryBranch(const RemoteBranchInfoPtr& info,
                              std::function<void()>&& success_handler) {}

}  // namespace detail
}  // namespace objects
