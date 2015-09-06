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
    EOFError() : Error(1000, "unknown_failure 1000") {}
};

/// Timeout error
class TimeoutError : public common::Error {
  public:
    /// Default constructor
    TimeoutError() : Error(1001, "generic_timeout_error") {}
};

/// Socket error
class SocketError : public common::Error {
  public:
    /// Default constructor
    SocketError() : Error(1002, "unknown_failure 1002") {}
};

/// Connect failed error
class ConnectFailedError : public common::Error {
  public:
    /// Default constructor
    ConnectFailedError() : Error(1003, "unknown_failure 1003") {}
};

/// DNS generic error
class DNSGenericError : public common::Error {
  public:
    /// Default constructor
    DNSGenericError() : Error(1004, "unknown_failure 1004") {}
};

/// Bad SOCKS version error
class BadSocksVersionError : public common::Error {
  public:
    /// Default constructor
    BadSocksVersionError() : Error(1005, "socks_error") {}
};

/// SOCKS address too long
class SocksAddressTooLongError : public common::Error {
  public:
    SocksAddressTooLongError() : Error(1006, "unknown_failure 1006") {}
};

/// SOCKS invalid port
class SocksInvalidPortError : public common::Error {
  public:
    SocksInvalidPortError() : Error(1007, "unknown_failure 1007") {}
};

/// SOCKS generic error
class SocksGenericError : public common::Error {
  public:
    SocksGenericError() : Error(1008, "socks_error") {}
};

/// EOL not found error
class EOLNotFoundError : public common::Error {
  public:
    EOLNotFoundError() : Error(1009, "unknown_failure 1009") {}
};

/// Line too long error
class LineTooLongError : public common::Error {
  public:
    LineTooLongError() : Error(1010, "unknown_failure 1010") {}
};

} // namespace net
} // namespace measurement_kit
#endif
