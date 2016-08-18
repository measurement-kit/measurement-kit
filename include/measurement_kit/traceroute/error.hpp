// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_TRACEROUTE_ERROR_HPP
#define MEASUREMENT_KIT_TRACEROUTE_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {
namespace traceroute {

MK_DEFINE_ERR(MK_ERR_TRACEROUTE(0), SocketCreateError, "")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(1), SetsockoptError, "")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(2), ProbeAlreadyPendingError, "")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(3), PayloadTooLongError, "")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(4), StorageInitError, "")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(5), BindError, "")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(6), EventNewError, "")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(7), SendtoError, "")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(8), NoProbePendingError, "")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(9), ClockGettimeError, "")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(10), EventAddError, "")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(11), SocketAlreadyClosedError, "")

} // namespace net
} // namespace mk
#endif
