// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_NET_ERROR_HPP
#define MEASUREMENT_KIT_NET_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {
namespace net {

MK_DEFINE_ERR(MK_ERR_NET(0), EofError, "")
MK_DEFINE_ERR(MK_ERR_NET(1), TimeoutError, "generic_timeout_error")
MK_DEFINE_ERR(MK_ERR_NET(2), SocketError, "")
MK_DEFINE_ERR(MK_ERR_NET(3), ConnectFailedError, "")
MK_DEFINE_ERR(MK_ERR_NET(4), DnsGenericError, "")
MK_DEFINE_ERR(MK_ERR_NET(5), BadSocksVersionError, "socks_error")
MK_DEFINE_ERR(MK_ERR_NET(6), SocksAddressTooLongError, "")
MK_DEFINE_ERR(MK_ERR_NET(7), SocksInvalidPortError, "")
MK_DEFINE_ERR(MK_ERR_NET(8), SocksGenericError, "socks_error")
MK_DEFINE_ERR(MK_ERR_NET(9), EOLNotFoundError, "")
MK_DEFINE_ERR(MK_ERR_NET(10), LineTooLongError, "")
MK_DEFINE_ERR(MK_ERR_NET(11), NetworkError, "")
MK_DEFINE_ERR(MK_ERR_NET(12), NoAvailableSocksAuthenticationError, "")
MK_DEFINE_ERR(MK_ERR_NET(13), SocksError, "")
MK_DEFINE_ERR(MK_ERR_NET(14), BadSocksReservedFieldError, "")
MK_DEFINE_ERR(MK_ERR_NET(15), BadSocksAtypeValueError, "")
MK_DEFINE_ERR(MK_ERR_NET(16), EvconnlistenerNewBindError, "")
MK_DEFINE_ERR(MK_ERR_NET(17), BuffereventSocketNewError, "")

class SSLInvalidCertificateError : public Error {
  public:
    SSLInvalidCertificateError(std::string msg)
        : Error(MK_ERR_NET(18), "ssl_invalid_certificate " + msg) {}
};

MK_DEFINE_ERR(MK_ERR_NET(19), SSLNoCertificateError, "ssl_no_certificate")
MK_DEFINE_ERR(MK_ERR_NET(20), SSLInvalidHostnameError, "ssl_invalid_hostname")

class SSLError : public Error {
  public:
    SSLError(std::string msg) : Error(MK_ERR_NET(21), "ssl_error " + msg) {}
};

MK_DEFINE_ERR(MK_ERR_NET(22), NotEnoughDataError, "")
MK_DEFINE_ERR(MK_ERR_NET(23), MissingCaBundlePathError, "")
MK_DEFINE_ERR(MK_ERR_NET(24), BrokenPipeError, "")

} // namespace net
} // namespace mk
#endif
