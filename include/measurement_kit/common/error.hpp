// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_ERROR_HPP
#define MEASUREMENT_KIT_COMMON_ERROR_HPP

#include <exception>
#include <iosfwd>
#include <string>

namespace measurement_kit {
namespace common {

/// An error that occurred
class Error : public std::exception {
  public:
    /// Constructor with error code and OONI error
    Error(int e, std::string ooe) : error_(e), ooni_error_(ooe) {}

    Error() : Error(0, "") {}               ///< Default constructor (no error)
    operator int() const { return error_; } ///< Cast to integer

    /// Equality operator
    bool operator==(int n) const { return error_ == n; }

    /// Equality operator
    bool operator==(Error e) const { return error_ == e.error_; }

    /// Unequality operator
    bool operator!=(int n) const { return error_ != n; }

    /// Unequality operator
    bool operator!=(Error e) const { return error_ != e.error_; }

    /// Return error as OONI error
    std::string as_ooni_error() { return ooni_error_; }

  private:
    int error_ = 0;
    std::string ooni_error_;
};

/// No error
class NoError : public Error {
  public:
    NoError() : Error() {} ///< Default constructor
};

/// Generic error
class GenericError : public Error {
  public:
    GenericError() : Error(1, "unknown_failure 1") {} ///< Default constructor
};

} // namespace common
} // namespace measurement_kit
#endif
