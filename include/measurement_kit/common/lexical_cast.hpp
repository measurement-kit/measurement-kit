// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_LEXICAL_CAST_HPP
#define MEASUREMENT_KIT_COMMON_LEXICAL_CAST_HPP

#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/error_or.hpp>
#include <sstream>

namespace mk {

template <typename To, typename From> To lexical_cast(From f) {
    std::stringstream ss;
    To value;
    ss << f;
    ss >> value;
    if (!ss.eof()) {
        throw ValueError(); // Not all input was converted
    }
    if (ss.fail()) {
        throw ValueError(); // Input format was wrong
    }
    return value;
}

template <typename To, typename From>
ErrorOr<To> lexical_cast_noexcept(From f) {
    try {
        return lexical_cast<To, From>(f);
    } catch (Error err) {
        return err;
    }
}

} // namespace mk
#endif
