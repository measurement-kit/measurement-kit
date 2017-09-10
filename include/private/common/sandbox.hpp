// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_COMMON_SANDBOX_HPP
#define PRIVATE_COMMON_SANDBOX_HPP

#include "private/common/maybe.hpp"
#include <measurement_kit/common/error.hpp>

namespace mk {

template <typename Callable> Error sandbox_for_errors(Callable fun) {
    Error error = NoError();
    try {
        fun();
    } catch (const Error &exc) {
        error = exc;
    }
    return error;
}

template <typename Callable>
Maybe<std::exception> sandbox_for_exceptions(Callable fun) {
    try {
        fun();
    } catch (std::exception &exc) {
        return std::move(exc);
    }
    return {};
}

} // namespace mk
#endif
