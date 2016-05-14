// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_TRACEROUTE_ERROR_HPP
#define MEASUREMENT_KIT_TRACEROUTE_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {
namespace traceroute {

MK_DEFINE_ERR(4000, SocketCreateError, "unknown_failure 4000")
MK_DEFINE_ERR(4001, SetsockoptError, "unknown_failure 4001")
MK_DEFINE_ERR(4002, ProbeAlreadyPendingError, "unknown_failure 4002")
MK_DEFINE_ERR(4003, PayloadTooLongError, "unknown_failure 4003")
MK_DEFINE_ERR(4004, StorageInitError, "unknown_failure 4004")
MK_DEFINE_ERR(4005, BindError, "unknown_failure 4005")
MK_DEFINE_ERR(4006, EventNewError, "unknown_failure 4006")
MK_DEFINE_ERR(4007, SendtoError, "unknown_failure 4007")
MK_DEFINE_ERR(4008, NoProbePendingError, "unknown_failure 4008")
MK_DEFINE_ERR(4009, ClockGettimeError, "unknown_failure 4009")
MK_DEFINE_ERR(4010, EventAddError, "unknown_failure 4010")
MK_DEFINE_ERR(4011, SocketAlreadyClosedError, "unknown_failure 4011")

} // namespace net
} // namespace mk
#endif
