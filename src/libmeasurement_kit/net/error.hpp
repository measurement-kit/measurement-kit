// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NET_ERROR_HPP
#define SRC_LIBMEASUREMENT_KIT_NET_ERROR_HPP

#include <measurement_kit/common.hpp>

#include <string>

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
 * Historically used, now removed values: [29, 58)
 */

MK_DEFINE_ERR(MK_ERR_NET(58), SslDirtyShutdownError, "ssl_dirty_shutdown")
MK_DEFINE_ERR(MK_ERR_NET(59), SslMissingHostnameError, "ssl_missing_hostname")

/*
 * Mapping between errno (Unix) / WSAGetLastError (Windows) values and
 * the corresponding strings. The strings we're using are backward compatible
 * with older versions of Measurement Kit. They are the same strings with
 * which such errors are named in the C++11 standard, BTW.
 *
 * Errors for which we don't have a Windows definition are included into a
 * separate list, because that simplifies their handling significantly.
 *
 * Errno names (first argument) are like their POSIX counterpart, with the
 * leading E (Unix) / WSAE (Windows) string missing.
 */

#define MK_NET_ERRNO_UNIX_ONLY(XX)                                             \
    XX(PIPE, broken_pipe)                                                      \
    XX(PROTO, protocol_error)

#define MK_NET_ERRNO(XX)                                                       \
    XX(AFNOSUPPORT, address_family_not_supported)                              \
    XX(ADDRINUSE, address_in_use)                                              \
    XX(ADDRNOTAVAIL, address_not_available)                                    \
    XX(ISCONN, already_connected)                                              \
    XX(FAULT, bad_address)                                                     \
    XX(BADF, bad_file_descriptor)                                              \
    XX(CONNABORTED, connection_aborted)                                        \
    XX(ALREADY, connection_already_in_progress)                                \
    XX(CONNREFUSED, connection_refused)                                        \
    XX(CONNRESET, connection_reset)                                            \
    XX(DESTADDRREQ, destination_address_required)                              \
    XX(HOSTUNREACH, host_unreachable)                                          \
    XX(INTR, interrupted)                                                      \
    XX(INVAL, invalid_argument)                                                \
    XX(MSGSIZE, message_size)                                                  \
    XX(NETDOWN, network_down)                                                  \
    XX(NETRESET, network_reset)                                                \
    XX(NETUNREACH, network_unreachable)                                        \
    XX(NOBUFS, no_buffer_space)                                                \
    XX(NOPROTOOPT, no_protocol_option)                                         \
    XX(NOTSOCK, not_a_socket)                                                  \
    XX(NOTCONN, not_connected)                                                 \
    XX(WOULDBLOCK, operation_would_block)                                      \
    XX(ACCES, permission_denied)                                               \
    XX(PROTONOSUPPORT, protocol_not_supported)                                 \
    XX(TIMEDOUT, timed_out)                                                    \
    XX(PROTOTYPE, wrong_protocol_type)

// Maps network error to corresponding OONI error. To this end it uses the
// above defined error listings by correctly setting the `XX` macro.
//
// TODO(bassosimone): write more unit tests for this functionality.
bool net_error_to_ooni_error(int code, std::string *str) noexcept;

} // namespace net
} // namespace mk
#endif
