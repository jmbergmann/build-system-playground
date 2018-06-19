#pragma once

#include "../config.h"
#include "../api/error.h"
#include "../utils/types.h"
#include "context.h"

#include <mutex>
#include <functional>
#include <queue>

namespace objects {

class SignalSet
    : public api::ExposedObjectT<SignalSet, api::ObjectType::kSignalSet> {
 public:
  enum Signals {
    kNoSignal = YOGI_SIG_NONE,
    kSigInt = YOGI_SIG_INT,
    kSigTerm = YOGI_SIG_TERM,
    kSigUsr1 = YOGI_SIG_USR1,
    kSigUsr2 = YOGI_SIG_USR2,
    kSigUsr3 = YOGI_SIG_USR3,
    kSigUsr4 = YOGI_SIG_USR4,
    kSigUsr5 = YOGI_SIG_USR5,
    kSigUsr6 = YOGI_SIG_USR6,
    kSigUsr7 = YOGI_SIG_USR7,
    kSigUsr8 = YOGI_SIG_USR8,
    kAllSignals = YOGI_SIG_ALL,
  };

  typedef std::function<void()> CleanupHandler;
  typedef std::function<void(const api::Error& res, Signals signal,
                             void* sigarg)>
      AwaitHandler;

  static void RaiseSignal(Signals signal, void* sigarg,
                          CleanupHandler cleanup_handler);

  SignalSet(ContextPtr context, Signals signals);
  virtual ~SignalSet();

  Signals GetSignals() const { return signals_; }
  void Await(AwaitHandler handler);
  void CancelAwait();

 private:
  struct SignalData {
    Signals signal;
    void* sigarg;
    int cnt;
    CleanupHandler cleanup_handler;
  };

  typedef std::shared_ptr<SignalData> SignalDataPtr;

  void OnSignalRaised(const SignalDataPtr& data);
  void DeliverNextSignal();

  const ContextPtr context_;
  const Signals signals_;

  std::mutex mutex_;
  AwaitHandler handler_;
  std::queue<SignalDataPtr> queue_;
};

typedef std::shared_ptr<SignalSet> SignalSetPtr;

YOGI_DEFINE_FLAG_OPERATORS(SignalSet::Signals)

}  // namespace objects
