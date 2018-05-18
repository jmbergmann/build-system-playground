#pragma once

#include "../../../config.h"
#include "../../../utils/socket.h"
#include "../../context.h"
#include "../../logger.h"
#include "branch_info.h"

#include <boost/asio.hpp>
#include <memory>
#include <functional>

namespace objects {
namespace detail {

class TcpBase : public std::enable_shared_from_this<TcpBase> {
 public:
  typedef std::function<void(RemoteBranchInfoPtr)> SuccessHandler;
  typedef std::function<void(const api::Error&, utils::TimedTcpSocketPtr)> ErrorHandler;

  TcpBase(ContextPtr context, LocalBranchInfoPtr info,
          SuccessHandler&& success_handler, ErrorHandler&& error_handler);
  virtual ~TcpBase() {}

  virtual void Start();

  template <typename TO>
  std::weak_ptr<TO> MakeWeakPtr() {
    return {std::static_pointer_cast<TO>(this->shared_from_this())};
  }

 protected:
  static const LoggerPtr& GetLogger() { return logger_; }
  const ContextPtr& GetContext() const { return context_; }
  const LocalBranchInfoPtr& GetInfo() const { return info_; }

  void StartInfoExchange(utils::TimedTcpSocketPtr socket);
  void CallErrorHandler(const api::Error& err, utils::TimedTcpSocketPtr socket);

 private:
  void SendInfo(utils::TimedTcpSocketPtr socket);
  void OnInfoSent(utils::TimedTcpSocketPtr socket);
  void StartReceiveInfoHeader(utils::TimedTcpSocketPtr socket);
  void OnInfoHeaderReceived(utils::TimedTcpSocketPtr socket,
                            const std::vector<char>& buffer);
  void StartReceiveInfoBody(RemoteBranchInfoPtr branch, std::size_t body_size);
  void OnInfoBodyReceived(RemoteBranchInfoPtr branch,
                          const std::vector<char>& buffer);

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const LocalBranchInfoPtr info_;
  const SuccessHandler success_handler_;
  const ErrorHandler error_handler_;
  std::vector<char> serialized_info_;
};

}  // namespace detail
}  // namespace objects
