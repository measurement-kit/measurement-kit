// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_SANDBOX_HPP
#define MEASUREMENT_KIT_COMMON_SANDBOX_HPP

#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/maybe.hpp>

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
        return {};
    } catch (std::exception &exc) {
        return std::move(exc);
    }
    /* NOTREACHED */
}

} // namespace mk
#endif
