// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_ERROR_OR_HPP
#define MEASUREMENT_KIT_COMMON_ERROR_OR_HPP

#include <measurement_kit/common/error.hpp>

#include <stdexcept>

namespace mk {

/// \brief `ErrorOr` wraps either a type or an `Error`. We often use ErrorOr in
/// synchronous code when we don't want to throw an exception. The following
/// pattern is frequently used in MK:
///
/// ```C++
/// ErrorOr<Result> result = get_result_noexcept();
/// if (!result) {
///     callback(result.as_error());
///     return;
/// }
/// use_result(*result);
/// ```
///
/// Attempting to access the value (for example using as_value()) when ErrorOr
/// wraps an error causes a std::runtime_error to be thrown.
///
/// ErrorOr firstly appeared in measurement-kit v0.2.0. originally named
/// Maybe. We changed the name later because we have seen that also LLVM has
/// a class called ErrorOr.
///
/// We may probably want to get rid of ErrorOr when we switch to C++17
/// where unpacking of returned tuple is made pleasant.
template <typename T> class ErrorOr {
  public:
    /// \brief The default constuctor creates a non-initialized ErrorOr. This
    /// means basically constructing it with an error and no value.
    ErrorOr() : ErrorOr{NotInitializedError(), {}} {}

    /// The constructor with error and value sets both.
    ErrorOr(Error error, T value) : error_(error), value_(value) {}

    /// Operator bool returns true if ErrorOr wraps a value.
    operator bool() const { return error_ == NoError(); }

#define XX                                                                     \
    if (error_) {                                                              \
        throw std::runtime_error("error_or_contains_error");                   \
    }                                                                          \
    return value_;

    /// \brief `as_value()` returns the value if the ErrorOr contains a
    /// value and throws std::runtime_error if ErrorOr contains an error.
    const T &as_value() const { XX }

    /// \brief `as_value()` returns the value if the ErrorOr contains a
    /// value and throws std::runtime_error if ErrorOr contains an error.
    T &as_value() { XX }

#undef XX

    /// \brief `as_error()` returns the contained error or NoError().
    Error as_error() const { return error_; }

    /// Operator `*` is a compact way to call as_value().
    const T &operator*() const { return as_value(); }

    /// Operator `*` is a compact way to call as_value().
    T &operator*() { return as_value(); }

#define XX                                                                     \
    if (error_) {                                                              \
        throw std::runtime_error("error_or_contains_error");                   \
    }                                                                          \
    return &value_;

    /// \brief Operator `->` allows to access the underlying value as a
    /// pointer, or throws std::runtime_error if ErrorOr wraps an error.
    const T *operator->() const { XX }

    /// \brief Operator `->` allows to access the underlying value as a
    /// pointer, or throws std::runtime_error if ErrorOr wraps an error.
    T *operator->() { XX }

#undef XX

  private:
    Error error_;
    T value_;
};

} // namespace mk
#endif
