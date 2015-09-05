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

#define MEASUREMENT_KIT_NET_ERROR_BASE 256

} // namespace common
} // namespace measurement_kit
#endif
