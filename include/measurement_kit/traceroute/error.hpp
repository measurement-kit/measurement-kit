// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#ifndef MEASUREMENT_KIT_TRACEROUTE_ERROR_HPP
#define MEASUREMENT_KIT_TRACEROUTE_ERROR_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace traceroute {

MK_DEFINE_ERR(MK_ERR_TRACEROUTE(0), SocketCreateError, "cannot_create_socket")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(1), SetsockoptError, "cannot_set_socket_options")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(2), ProbeAlreadyPendingError, "response_already_pending")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(3), PayloadTooLongError, "payload_too_large")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(4), StorageInitError, "cannot_initialize_socket_address")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(5), BindError, "cannot_bind_socket")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(6), EventNewError, "event_new_error")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(7), SendtoError, "cannot_send_packet")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(8), NoProbePendingError, "no_probe_is_pending")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(9), ClockGettimeError, "cannot_get_current_time")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(10), EventAddError, "event_add_error")
MK_DEFINE_ERR(MK_ERR_TRACEROUTE(11), SocketAlreadyClosedError, "socket_already_closed")

} // namespace net
} // namespace mk
#endif
