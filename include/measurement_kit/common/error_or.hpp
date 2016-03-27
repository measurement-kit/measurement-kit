// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_ERROR_OR_HPP
#define MEASUREMENT_KIT_COMMON_ERROR_OR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {

template <typename T> class ErrorOr {
  public:
    ErrorOr() : error_(NotInitializedError()) {}

    ErrorOr(T value) : value_(value) {}

    ErrorOr(Error error) : error_(error) {}

    ErrorOr(Error error, T value) : error_(error), value_(value) {}

    operator bool() const { return error_ == NoError(); }

    T &as_value() {
        if (error_ != 0) {
            throw error_;
        }
        return value_;
    }

    Error as_error() const { return error_; }

    T &operator*() { return as_value(); }

    T *operator->() {
        if (error_ != 0) {
            throw error_;
        }
        return &value_;
    }

  private:
    Error error_;
    T value_;
};

} // namespace mk
#endif
