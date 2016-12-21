// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../portable/citrus_adapt.h"
#include <measurement_kit/common.hpp>

namespace mk {

bool is_valid_utf8(std::string s) {
    _utf8_state state = {};
    const char *src = s.data();
    size_t d = mk_citrus_utf8_ctype_mbsnrtowcs(nullptr, &src, s.size(), 0,
                                               (mbstate_t *)&state);
    return d == (size_t)-1;
}

} // namespace mk
