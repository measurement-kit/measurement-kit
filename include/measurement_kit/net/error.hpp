// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_NET_ERROR_HPP
#define MEASUREMENT_KIT_NET_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {
namespace net {

MK_DEFINE_ERR(1000, EofError, "")
MK_DEFINE_ERR(1001, TimeoutError, "generic_timeout_error")
MK_DEFINE_ERR(1002, SocketError, "")
MK_DEFINE_ERR(1003, ConnectFailedError, "")
MK_DEFINE_ERR(1004, DnsGenericError, "")
MK_DEFINE_ERR(1005, BadSocksVersionError, "socks_error")
MK_DEFINE_ERR(1006, SocksAddressTooLongError, "")
MK_DEFINE_ERR(1007, SocksInvalidPortError, "")
MK_DEFINE_ERR(1008, SocksGenericError, "socks_error")
MK_DEFINE_ERR(1009, EOLNotFoundError, "")
MK_DEFINE_ERR(1010, LineTooLongError, "")
MK_DEFINE_ERR(1011, NetworkError, "")
MK_DEFINE_ERR(1012, NoAvailableSocksAuthenticationError, "")
MK_DEFINE_ERR(1013, SocksError, "")
MK_DEFINE_ERR(1014, BadSocksReservedFieldError, "")
MK_DEFINE_ERR(1015, BadSocksAtypeValueError, "")
MK_DEFINE_ERR(1016, EvconnlistenerNewBindError, "")
MK_DEFINE_ERR(1017, BuffereventSocketNewError, "")

class SSLInvalidCertificateError : public Error {
  public:
    SSLInvalidCertificateError(std::string msg)
        : Error(1018, "ssl_invalid_certificate " + msg) {}
};

MK_DEFINE_ERR(1019, SSLNoCertificateError, "ssl_no_certificate")
MK_DEFINE_ERR(1020, SSLInvalidHostnameError, "ssl_invalid_hostname")

class SSLError : public Error {
  public:
    SSLError(std::string msg) : Error(1021, "ssl_error " + msg) {}
};

MK_DEFINE_ERR(1022, NotEnoughDataError, "")
MK_DEFINE_ERR(1023, MissingCaBundlePathError, "")
MK_DEFINE_ERR(1024, BrokenPipeError, "")

} // namespace net
} // namespace mk
#endif
