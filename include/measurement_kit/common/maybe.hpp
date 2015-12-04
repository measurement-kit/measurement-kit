// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_MAYBE_HPP
#define MEASUREMENT_KIT_COMMON_MAYBE_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {

/// Maybe contains a value of type T
template <typename T> class Maybe {
  public:
    /// Empty constructor
    Maybe() :  error_(MaybeNotInitializedError()) {}

    /// Constructor with value
    Maybe(T value) : Maybe(NoError(), value) {}

    /// Constructor with error and value
    Maybe(Error error, T value) : error_(error), value_(value) {}

    /// Whether we hold a value
    operator bool() { return error_ == NoError(); }

    /// Access maybe value as T
    T as_value() {
        if (error_ != 0) throw error_;
        return value_;
    }

    /// Access maybe value as Error
    Error as_error() { return error_; }

  private:
    Error error_;
    T value_;
};

} // namespace mk
#endif
