// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_NET_ERROR_HPP
#define MEASUREMENT_KIT_NET_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {
namespace net {

MK_DEFINE_ERR(1000, EofError, "unknown_failure 1000")
MK_DEFINE_ERR(1001, TimeoutError, "generic_timeout_error")
MK_DEFINE_ERR(1002, SocketError, "unknown_failure 1002")
MK_DEFINE_ERR(1003, ConnectFailedError, "unknown_failure 1003")
MK_DEFINE_ERR(1004, DnsGenericError, "unknown_failure 1004")
MK_DEFINE_ERR(1005, BadSocksVersionError, "socks_error")
MK_DEFINE_ERR(1006, SocksAddressTooLongError, "unknown_failure 1006")
MK_DEFINE_ERR(1007, SocksInvalidPortError, "unknown_failure 1007")
MK_DEFINE_ERR(1008, SocksGenericError, "socks_error")
MK_DEFINE_ERR(1009, EOLNotFoundError, "unknown_failure 1009")
MK_DEFINE_ERR(1010, LineTooLongError, "unknown_failure 1010")
MK_DEFINE_ERR(1011, NetworkError, "unknown_failure 1011")
MK_DEFINE_ERR(1012, NoAvailableSocksAuthenticationError, "unknown_failure 1012")
MK_DEFINE_ERR(1013, SocksError, "unknown_failure 1013")
MK_DEFINE_ERR(1014, BadSocksReservedFieldError, "unknown_failure 1014")
MK_DEFINE_ERR(1015, BadSocksAtypeValueError, "unknown_failure 1015")
MK_DEFINE_ERR(1016, EvconnlistenerNewBindError, "unknown_failure 1016")
MK_DEFINE_ERR(1017, BuffereventSocketNewError, "unknown_failure 1017")

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

MK_DEFINE_ERR(1022, NotEnoughDataError, "unknown_failure 1022")
MK_DEFINE_ERR(1023, MissingCaBundlePathError, "unknown_failure 1023")
MK_DEFINE_ERR(1024, BrokenPipeError, "unknown_failure 1024")

} // namespace net
} // namespace mk
#endif
