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

    /// Unequality operator
    bool operator!=(int n) const { return error_ != n; }

  private:
    int error_ = 0;
};

/// A generic error
class GenericError : public Error {
  public:
    GenericError() : Error(1) {} ///< Default constructor
};

} // namespace common
} // namespace measurement_kit
#endif
