// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_TRACEROUTE_ERROR_HPP
#define MEASUREMENT_KIT_TRACEROUTE_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {
namespace traceroute {

MK_DEFINE_ERR(4000, SocketCreateError, "")
MK_DEFINE_ERR(4001, SetsockoptError, "")
MK_DEFINE_ERR(4002, ProbeAlreadyPendingError, "")
MK_DEFINE_ERR(4003, PayloadTooLongError, "")
MK_DEFINE_ERR(4004, StorageInitError, "")
MK_DEFINE_ERR(4005, BindError, "")
MK_DEFINE_ERR(4006, EventNewError, "")
MK_DEFINE_ERR(4007, SendtoError, "")
MK_DEFINE_ERR(4008, NoProbePendingError, "")
MK_DEFINE_ERR(4009, ClockGettimeError, "")
MK_DEFINE_ERR(4010, EventAddError, "")
MK_DEFINE_ERR(4011, SocketAlreadyClosedError, "")

} // namespace net
} // namespace mk
#endif
