#pragma once

#include "../../config.h"

namespace objects {
namespace detail {

class AdvertisingReceiver
    : public std::enable_shared_from_this<AdvertisingReceiver> {};

typedef std::shared_ptr<AdvertisingReceiver> AdvertisingReceiverPtr;

}  // namespace detail
}  // namespace objects
