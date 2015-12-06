// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_DNS_ERROR_HPP
#define MEASUREMENT_KIT_DNS_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {
namespace dns {

/// Format error
class FormatError : public Error {
  public:
    /// Default constructor
    FormatError() : Error(2000, "dns_lookup_error") {}
};

/// Server failed error
class ServerFailedError : public Error {
  public:
    /// Default constructor
    ServerFailedError() : Error(2001, "dns_lookup_error") {}
};

/// Not exist error
class NotExistError : public Error {
  public:
    /// Default constructor
    NotExistError() : Error(2002, "dns_lookup_error") {}
};

/// Not implemented error
class NotImplementedError : public Error {
  public:
    /// Default constructor
    NotImplementedError() : Error(2003, "dns_lookup_error") {}
};

/// Refused error
class RefusedError : public Error {
  public:
    /// Default constructor
    RefusedError() : Error(2004, "dns_lookup_error") {}
};

/// Truncated error
class TruncatedError : public Error {
  public:
    /// Default constructor
    TruncatedError() : Error(2005, "dns_lookup_error") {}
};

/// Uknown error
class UnknownError : public Error {
  public:
    UnknownError() : Error(2006, "unknown_failure 2006") {}
};

/// Timeout error
class TimeoutError : public Error {
  public:
    TimeoutError() : Error(2007, "generic_timeout_error") {}
};

/// Shutdown error
class ShutdownError : public Error {
  public:
    ShutdownError() : Error(2008, "unknown_failure 2008") {}
};

/// Cancel error
class CancelError : public Error {
  public:
    CancelError() : Error(2009, "unknown_failure 2009") {}
};

/// No data error
class NoDataError : public Error {
  public:
    NoDataError() : Error(2010, "dns_lookup_error") {}
};

/// Invalid IPv4 error
class InvalidIPv4AddressError : public Error {
  public:
    InvalidIPv4AddressError() : Error(2011, "unknown_failure 2011") {}
};

/// Invalid IPv6 error
class InvalidIPv6AddressError : public Error {
  public:
    InvalidIPv6AddressError() : Error(2012, "unknown_failure 2012") {}
};

/// Unsupported class error
class UnsupportedClassError : public Error {
  public:
    UnsupportedClassError() : Error(2013, "unknown_failure 2013") {}
};

/// Invalid name for PTR query error
class InvalidNameForPTRError : public Error {
  public:
    InvalidNameForPTRError() : Error(2014, "unknown_failure 2014") {}
};

/// Resolver error
class ResolverError : public Error {
  public:
    ResolverError() : Error(2015, "unknown_failure 2015") {}
};

/// Unsupported type error
class UnsupportedTypeError : public Error {
  public:
    UnsupportedTypeError() : Error(2016, "unknown_failure 2016") {}
};

Error dns_error(int code); ///< Map evdns code to proper error

} // namespace dns
} // namespace mk
#endif
