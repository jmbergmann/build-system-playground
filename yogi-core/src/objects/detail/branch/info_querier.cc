#include "info_querier.h"

namespace objects {
namespace detail {
namespace branch {

const LoggerPtr InfoQuerier::logger_ =
    Logger::CreateStaticInternalLogger("Branch");

InfoQuerier::InfoQuerier(std::chrono::nanoseconds timeout,
                         std::chrono::nanoseconds retry_time)
    : timeout_(timeout), retry_time_(retry_time) {}

void InfoQuerier::QueryBranch(
    const BranchInfoPtr& info,
    std::function<void(const utils::TimedTcpSocketPtr& socket)>&&
        success_handler) {}

}  // namespace branch
}  // namespace detail
}  // namespace objects
