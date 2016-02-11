// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_COMMON_MOCK_HPP
#define SRC_COMMON_MOCK_HPP

#define MK_MOCK(foo) decltype(foo) mocked_func = foo
#define MK_MOCK_ALLOC(foo, bar)                                                \
    decltype(foo) mocked_alloc = foo, decltype(bar) mocked_dealloc = bar

#endif
