#ifndef MEASUREMENT_KIT_MKDATA_H
#define MEASUREMENT_KIT_MKDATA_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/// mkdata_t is a piece of possibly UTF-8 binary data
typedef struct mkdata mkdata_t;

/// mkdata_new creates a new mkdata_t instance
mkdata_t *mkdata_new(void);

/// mkdata_set copies data from the provided vector into @p data
void mkdata_set(mkdata_t *data, const uint8_t *base, size_t count);

/// mkdata_contains_valid_utf8 tells you whether @p data contains valid UTF-8
int64_t mkdata_contains_valid_utf8(mkdata_t *data);

/// mkdata_get_base64 gives you a base64 encoded representation of the data
/// contained inside of @p data. Returns a boolean-ish value, where true (i.e.
/// nonzero) means success and false (zero) means failure.
int64_t mkdata_get_base64(mkdata_t *data, const uint8_t **p, size_t *n);

/// mkdata_delete delets the @p data instance.
void mkdata_delete(mkdata_t *data);

#ifdef __cplusplus
}  // extern "C"

#include <assert.h>

#include <memory>
#include <string>

/// mkdata_deleter is a deleter for mkdata_t.
struct mkdata_deleter {
  void operator()(mkdata_t *s) { mkdata_delete(s); }
};

/// mkdata_uptr is a unique pointer to a mkdata_t.
using mkdata_uptr = std::unique_ptr<mkdata_t, mkdata_deleter>;

/// mkdata_movein moves data from the string insider the mkdata_t instance.
void mkdata_movein(mkdata_t *data, std::string &&s);

/// mkdata_move_base64 converts to base64 and moves the base64 encoded data
/// into the @p rv string. Returns true on success, false on failure.
int64_t mkdata_move_base64(mkdata_t *data, std::string *rv);

/// mkdata_moveout moves the internal data into the returned string.
std::string mkdata_moveout(mkdata_t *data);

#ifdef MKDATA_INLINE_IMPL

struct mkdata {
  std::string data;
  std::string base64;
};

mkdata_t *mkdata_new(void) { return new mkdata_t{}; }

void mkdata_set(mkdata_t *data, const uint8_t *base, size_t count) {
  if (data != nullptr && base != nullptr) {
    mkdata_movein(data, std::string{(char *)base, count});
  }
}

// === BEGIN{ http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ ===
// clang-format off
//
// Portions Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

static const uint8_t mkdata_utf8d[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
  8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
  0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
  0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
  0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
  1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
  1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
  1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

static uint32_t inline
mkdata_decode_utf8(uint32_t* state, uint32_t* codep, uint32_t byte) {
  uint32_t type = mkdata_utf8d[byte];

  *codep = (*state != UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = mkdata_utf8d[256 + *state*16 + type];
  return *state;
}

// clang-format on
// === }END http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ ===

int64_t mkdata_contains_valid_utf8(mkdata_t *data) {
  if (data == nullptr) return false;
  uint32_t codepoint{};
  uint32_t state{};
  for (size_t i = 0; i < data->data.size(); ++i) {
    (void)mkdata_decode_utf8(&state, &codepoint, (uint8_t)data->data[i]);
  }
  return state == UTF8_ACCEPT;
}

// === BEGIN{ René Nyffenegger BASE64 code ===
/*-
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

static inline std::string mkdata_b64_encode(const uint8_t *base, size_t len) {
    static const std::string b64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                         "abcdefghijklmnopqrstuvwxyz"
                                         "0123456789+/";
    std::string res;
    uint8_t in[3];
    uint8_t out[4];
    int state = 0;
    auto copy = [&]() {
        // The following code has been reorganized to avoid -Wconversion
        // -Werror error generated by Clang on macOS, as well as to avoid
        // -Werror=conversion as generated by GCC on GNU/Linux
        {
          auto n = (in[0] & 0xfc) >> 2;
          assert(n <= 0xff);
          out[0] = (uint8_t)n;
        }
        {
          auto n = ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4);
          assert(n <= 0xff);
          out[1] = (uint8_t)n;
        }
        {
          auto n = ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6);
          assert(n <= 0xff);
          out[2] = (uint8_t)n;
        }
        {
          auto n = in[2] & 0x3f;
          assert(n <= 0xff);
          out[3] = (uint8_t)n;
        }
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
// === }END René Nyffenegger BASE64 code ===

int64_t mkdata_get_base64(mkdata_t *data, const uint8_t **p, size_t *n) {
  if (data == nullptr || p == nullptr || n == nullptr) return false;
  data->base64 = mkdata_b64_encode((const uint8_t *)data->data.c_str(),
                                   data->data.size());
  *p = (const uint8_t *)data->base64.c_str();
  *n = data->base64.size();
  return true;
}

void mkdata_delete(mkdata_t *data) { delete data; }

void mkdata_movein(mkdata_t *data, std::string &&s) {
  if (data != nullptr) {
    std::swap(s, data->data);
    data->base64 = "";
  }
}

int64_t mkdata_move_base64(mkdata_t *data, std::string *rv) {
  if (data == nullptr || rv == nullptr) return false;
  const uint8_t *base_ignored = nullptr;
  size_t length_ignored = 0;
  int64_t okay = mkdata_get_base64(data, &base_ignored, &length_ignored);
  if (okay) std::swap(*rv, data->base64);
  return true;
}

std::string mkdata_moveout(mkdata_t *data) {
  std::string s;
  if (data != nullptr) std::swap(data->data, s);
  return s;
}

#endif  // MKDATA_INLINE_IMPL
#endif  // __cplusplus
#endif  // MEASUREMENT_KIT_MKDATA_H
