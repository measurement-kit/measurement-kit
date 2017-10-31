// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_COMMON_MOCK_HPP
#define PRIVATE_COMMON_MOCK_HPP

/*
Simplifies life when you use templates for mocking APIs because
allows you to write the following:

    template <MK_MOCK(event_base_new)>
    void foobar() {
        event_base *p = event_base_new();
    }

Which is arguably faster than writing the following:

    template <decltype(event_base_new) event_base_new = ::event_base_new>
    void foobar() { ...
*/
#define MK_MOCK(name_) MK_MOCK_AS(name_, name_)

// Similar to MK_MOCK but with alias
#define MK_MOCK_AS(name_, alias_) decltype(name_) alias_ = name_

#endif
