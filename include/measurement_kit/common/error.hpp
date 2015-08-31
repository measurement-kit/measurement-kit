// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_ERROR_HPP
#define MEASUREMENT_KIT_COMMON_ERROR_HPP

namespace measurement_kit {
namespace common {

/// An error that occurred
class Error {
  public:
    Error() : Error(0) {}                   ///< Default constructor (no error)
    Error(int e) { error_ = e; }            ///< Constructor with error code
    operator int() const { return error_; } ///< Cast to integer

    /// Equality operator
    bool operator==(int n) const { return error_ == n; }

    /// Equality operator
    bool operator==(Error e) const { return error_ == e.error_; }

    /// Unequality operator
    bool operator!=(int n) const { return error_ != n; }

    /// Unequality operator
    bool operator!=(Error e) const { return error_ != e.error_; }

  private:
    int error_ = 0;
};

/// 1 - Generic error
class GenericError : public Error {
  public:
    GenericError() : Error(1) {} ///< Default constructor
};

/// 2 - EOF error
class EOFError : public Error {
  public:
    EOFError() : Error(2) {} ///< Default constructor
};

/// 3 - Timeout error
class TimeoutError : public Error {
  public:
    TimeoutError() : Error(3) {} ///< Default constructor
};

/// 4 - Socket error
class SocketError : public Error {
  public:
    SocketError() : Error(4) {} ///< Default constructor
};

/// 5 - Connect failed error
class ConnectFailedError : public Error {
  public:
    ConnectFailedError() : Error(5) {} ///< Default constructor
};

/// 6 - DNS generic error
class DNSGenericError : public Error {
  public:
    DNSGenericError() : Error(6) {} ///< Default constructor
};

/// 7 - Bad SOCKS version error
class BadSocksVersionError : public Error {
  public:
    BadSocksVersionError() : Error(7) {} ///< Default constructor
};

/// 8 - SOCKS address too long
class SocksAddressTooLongError : public Error {
  public:
    SocksAddressTooLongError() : Error(8) {} ///< Default constructor
};

/// 9 - SOCKS invalid port
class SocksInvalidPortError : public Error {
  public:
    SocksInvalidPortError() : Error(9) {} ///< Default constructor
};

/// 10 - SOCKS generic error
class SocksGenericError : public Error {
  public:
    SocksGenericError() : Error(10) {} ///< Default constructor
};

/// 11 - EOL not found error
class EOLNotFoundError : public Error {
  public:
    EOLNotFoundError() : Error(11) {} ///< Default constructor
};

/// 12 - Line too long error
class LineTooLongError : public Error {
  public:
    LineTooLongError() : Error(12) {} ///< Default constructor
};

} // namespace common
} // namespace measurement_kit
#endif
