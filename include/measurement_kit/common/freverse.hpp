/*
 * Part of measurement-kit <https://measurement-kit.github.io/>.
 * Measurement-kit is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 * =============================================================
 * Based on <https://github.com/aherrmann/unpacking_tuples_examples>:
 * Portions Copyright (c) 2016 Andreas Herrmann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef MEASUREMENT_KIT_COMMON_FREVERSE_HPP
#define MEASUREMENT_KIT_COMMON_FREVERSE_HPP

#include <tuple>
#include <utility>

namespace mk {

template <class F, std::size_t... Is>
constexpr auto index_apply_impl_(F f, std::index_sequence<Is...>) {
    return f(std::integral_constant<std::size_t, Is>{}...);
}

template <std::size_t N, class F> constexpr auto index_apply_(F f) {
    return index_apply_impl_(f, std::make_index_sequence<N>{});
}

template <class Tuple> constexpr auto freverse(Tuple t) {
    return index_apply_<std::tuple_size<Tuple>{}>([&](auto... Is) {
        return std::make_tuple(
            std::get<std::tuple_size<Tuple>{} - 1 - Is>(t)...);
    });
}

} // namespace mk
#endif
