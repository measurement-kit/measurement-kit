// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_DETAIL_ENCODING_HPP
#define MEASUREMENT_KIT_COMMON_DETAIL_ENCODING_HPP

#include <measurement_kit/common/error.hpp>
#include "private/portable/citrus_adapt.h"
#include <measurement_kit/common/detail/base64_encode_.hpp>

#include <cassert>

namespace mk {

inline Error is_valid_utf8_string(const std::string &s) {
    _utf8_state state = {};
    size_t r = 1;
    for (char c : s) {
        r = mk_utf8_mbrtowc(nullptr, &c, 1, (mbstate_t *)&state);
        if (r == (size_t)-2) {
            // We're in the middle of a multibyte sequence
            continue;
        }
        if (r == (size_t)-1) {
            // The sequence is invalid
            return IllegalSequenceError();
        }
        if (r == 0) {
            // We were not expecting a L'\0' at this point
            return UnexpectedNullByteError();
        }
        /*
         * Because we parse each character independently, mbrtowc() should
         * tell us that we have consumed one character after each call.
         */
        assert(r == 1);
    }
    assert(r == (size_t)-2 || r == 1);
    if (r == (size_t)-2) {
        return IncompleteUtf8SequenceError();
    }
    return NoError();
}

inline std::string base64_encode(std::string s) {
    return base64_encode_impl((const uint8_t *)s.data(), s.size());
}

} // namespace mk
#endif
