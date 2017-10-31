/*-
 * Part of measurement-kit <https://measurement-kit.github.io/>.
 * Measurement-kit is free software under the BSD license. See AUTHORS
 * and LICENSE for more information on the copying conditions.
 * =============================================================================
 * base64.cpp and base64.h
 *
 * Portions Copyright (C) 2004-2008 René Nyffenegger
 *
 * This source code is provided 'as-is', without any express or implied
 * warranty. In no event will the author be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this source code must not be misrepresented; you must not
 *    claim that you wrote the original source code. If you use this source code
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original source code.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * René Nyffenegger rene.nyffenegger@adp-gmbh.ch
 */

#include <cstdint>
#include "private/common/encoding.hpp"
#include <string>

namespace mk {

static inline std::string base64_encode(const uint8_t *base, size_t len) {
    static const std::string b64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                         "abcdefghijklmnopqrstuvwxyz"
                                         "0123456789+/";
    std::string res;
    uint8_t in[3];
    uint8_t out[4];
    int state = 0;
    auto copy = [&]() {
        out[0] = (in[0] & 0xfc) >> 2;
        out[1] = ((in[0] & 0x03) << 4) + ((in[1] & 0xf0) >> 4);
        out[2] = ((in[1] & 0x0f) << 2) + ((in[2] & 0xc0) >> 6);
        out[3] = in[2] & 0x3f;
        for (int idx = 0; idx < 4; ++idx) {
            res += (idx <= state) ? b64_table[out[idx]] : '=';
        }
    };
    while (len-- > 0) {
        in[state++] = *(base++);
        if (state == 3) {
            copy();
            state = 0;
        }
    }
    if (state != 0) {
        for (int idx = state; idx < 3; ++idx) {
            in[idx] = '\0';
        }
        copy();
    }
    return res;
}

std::string base64_encode(std::string s) {
    return base64_encode((uint8_t *)s.data(), s.size());
}

} // namespace mk
