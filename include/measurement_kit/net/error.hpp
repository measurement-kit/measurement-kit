// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_NET_ERROR_HPP
#define MEASUREMENT_KIT_NET_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace measurement_kit {
namespace net {

/// EOF error
class EOFError : public common::Error {
  public:
    /// Default constructor
    EOFError() : Error(MEASUREMENT_KIT_NET_ERROR_BASE + 1) {}
};

/// Timeout error
class TimeoutError : public common::Error {
  public:
    /// Default constructor
    TimeoutError() : Error(MEASUREMENT_KIT_NET_ERROR_BASE + 2) {}
};

/// Socket error
class SocketError : public common::Error {
  public:
    /// Default constructor
    SocketError() : Error(MEASUREMENT_KIT_NET_ERROR_BASE + 3) {}
};

/// Connect failed error
class ConnectFailedError : public common::Error {
  public:
    /// Default constructor
    ConnectFailedError() : Error(MEASUREMENT_KIT_NET_ERROR_BASE + 4) {}
};

/// DNS generic error
class DNSGenericError : public common::Error {
  public:
    /// Default constructor
    DNSGenericError() : Error(MEASUREMENT_KIT_NET_ERROR_BASE + 5) {}
};

/// Bad SOCKS version error
class BadSocksVersionError : public common::Error {
  public:
    /// Default constructor
    BadSocksVersionError() : Error(MEASUREMENT_KIT_NET_ERROR_BASE + 6) {}
};

/// SOCKS address too long
class SocksAddressTooLongError : public common::Error {
  public:
    SocksAddressTooLongError() : Error(MEASUREMENT_KIT_NET_ERROR_BASE + 7) {}
};

/// SOCKS invalid port
class SocksInvalidPortError : public common::Error {
  public:
    SocksInvalidPortError() : Error(MEASUREMENT_KIT_NET_ERROR_BASE + 8) {}
};

/// SOCKS generic error
class SocksGenericError : public common::Error {
  public:
    SocksGenericError() : Error(MEASUREMENT_KIT_NET_ERROR_BASE + 9) {}
};

/// EOL not found error
class EOLNotFoundError : public common::Error {
  public:
    EOLNotFoundError() : Error(MEASUREMENT_KIT_NET_ERROR_BASE + 10) {}
};

/// Line too long error
class LineTooLongError : public common::Error {
  public:
    LineTooLongError() : Error(MEASUREMENT_KIT_NET_ERROR_BASE + 11) {}
};

} // namespace net
} // namespace measurement_kit
#endif
