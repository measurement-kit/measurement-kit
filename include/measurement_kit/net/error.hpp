// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NET_ERROR_HPP
#define MEASUREMENT_KIT_NET_ERROR_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace net {

MK_DEFINE_ERR(MK_ERR_NET(0), EofError, "eof_error")
using TimeoutError = mk::TimeoutError; /* Was: MK_ERR_NET(1) */
/* Unused: MK_ERR_NET(2) */
MK_DEFINE_ERR(MK_ERR_NET(3), ConnectFailedError, "connect_error")
MK_DEFINE_ERR(MK_ERR_NET(4), DnsGenericError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_NET(5), BadSocksVersionError, "socks_error")
MK_DEFINE_ERR(MK_ERR_NET(6), SocksAddressTooLongError, "socks_address_too_long")
MK_DEFINE_ERR(MK_ERR_NET(7), SocksInvalidPortError, "socks_invalid_port")
MK_DEFINE_ERR(MK_ERR_NET(8), SocksGenericError, "socks_error")
MK_DEFINE_ERR(MK_ERR_NET(9), EOLNotFoundError, "eol_not_found")
MK_DEFINE_ERR(MK_ERR_NET(10), LineTooLongError, "line_too_long")
MK_DEFINE_ERR(MK_ERR_NET(11), NetworkError, "generic_network_error")
using SocketError = NetworkError; /* Alias */
MK_DEFINE_ERR(MK_ERR_NET(12), NoAvailableSocksAuthenticationError, "socks_no_available_authentication")
MK_DEFINE_ERR(MK_ERR_NET(13), SocksError, "socks_error")
MK_DEFINE_ERR(MK_ERR_NET(14), BadSocksReservedFieldError, "socks_bad_reserved_field")
MK_DEFINE_ERR(MK_ERR_NET(15), BadSocksAtypeValueError, "socks_bad_address_type")
MK_DEFINE_ERR(MK_ERR_NET(16), EvconnlistenerNewBindError, "bind_error")
MK_DEFINE_ERR(MK_ERR_NET(17), BuffereventSocketNewError, "bufferevent_new_error")
MK_DEFINE_ERR(MK_ERR_NET(18), SslInvalidCertificateError,
              "ssl_invalid_certificate")
MK_DEFINE_ERR(MK_ERR_NET(19), SslNoCertificateError, "ssl_no_certificate")
MK_DEFINE_ERR(MK_ERR_NET(20), SslInvalidHostnameError, "ssl_invalid_hostname")
MK_DEFINE_ERR(MK_ERR_NET(21), SslError, "ssl_error")
MK_DEFINE_ERR(MK_ERR_NET(22), NotEnoughDataError, "not_enough_data")
MK_DEFINE_ERR(MK_ERR_NET(23), MissingCaBundlePathError, "missing_ca_bundle_path")
/* Error code not used; was the old BrokenPipeError */
MK_DEFINE_ERR(MK_ERR_NET(25), SslNewError, "ssl_new_error")
MK_DEFINE_ERR(MK_ERR_NET(26), SslCtxNewError, "ssl_ctx_new_error")
MK_DEFINE_ERR(MK_ERR_NET(27), SslCtxLoadVerifyLocationsError, "ssl_ctx_load_verify_locations_error")
MK_DEFINE_ERR(MK_ERR_NET(28), SslCtxLoadVerifyMemError, "ssl_ctx_load_verify_mem_error")

/*
 * Maps std::errc from <system_error>
 *
 * Note that EAGAIN is not handled here but in the .cpp file
 * and gets mapped to EWOULDBLOCK unconditionally like most modern
 * Unix systems already do in their <errno.h> or <sys/errno.h>.
 */
#define MK_NET_ERRORS_XX                                                   \
    XX(29, AddressFamilyNotSupportedError, address_family_not_supported)   \
    XX(30, AddressInUseError, address_in_use)                              \
    XX(31, AddressNotAvailableError, address_not_available)                \
    XX(32, AlreadyConnectedError, already_connected)                       \
    XX(33, BadAddressError, bad_address)                                   \
    XX(34, BadFileDescriptorError, bad_file_descriptor)                    \
    XX(35, BrokenPipeError, broken_pipe)                                   \
    XX(36, ConnectionAbortedError, connection_aborted)                     \
    XX(37, ConnectionAlreadyInProgressError,                               \
       connection_already_in_progress)                                     \
    XX(38, ConnectionRefusedError, connection_refused)                     \
    XX(39, ConnectionResetError, connection_reset)                         \
    XX(40, DestinationAddressRequiredError, destination_address_required)  \
    XX(41, HostUnreachableError, host_unreachable)                         \
    XX(42, InterruptedError, interrupted)                                  \
    XX(43, InvalidArgumentError, invalid_argument)                         \
    XX(44, MessageSizeError, message_size)                                 \
    XX(45, NetworkDownError, network_down)                                 \
    XX(46, NetworkResetError, network_reset)                               \
    XX(47, NetworkUnreachableError, network_unreachable)                   \
    XX(48, NoBufferSpaceError, no_buffer_space)                            \
    XX(49, NoProtocolOptionError, no_protocol_option)                      \
    XX(50, NotASocketError, not_a_socket)                                  \
    XX(51, NotConnectedError, not_connected)                               \
    XX(52, OperationWouldBlockError, operation_would_block)                \
    XX(53, PermissionDeniedError, permission_denied)                       \
    XX(54, ProtocolErrorError, protocol_error)                             \
    XX(55, ProtocolNotSupportedError, protocol_not_supported)              \
    XX(56, TimedOutError, timed_out)                                       \
    XX(57, WrongProtocolTypeError, wrong_protocol_type)

#define XX(_code_, _name_, _descr_)                                        \
    MK_DEFINE_ERR(MK_ERR_NET(_code_), _name_, #_descr_)
MK_NET_ERRORS_XX
#undef XX

MK_DEFINE_ERR(MK_ERR_NET(58), SslDirtyShutdownError, "ssl_dirty_shutdown")
MK_DEFINE_ERR(MK_ERR_NET(59), SslMissingHostnameError, "ssl_missing_hostname")

} // namespace net
} // namespace mk
#endif
