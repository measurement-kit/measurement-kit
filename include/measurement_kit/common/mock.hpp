// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_MOCK_HPP
#define MEASUREMENT_KIT_COMMON_MOCK_HPP

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
#define MK_MOCK(name_) decltype(name_) name_ = name_

// Same as MK_MOCK but with suffix
#define MK_MOCK_SUFFIX(name_, suffix_) decltype(name_) name_##_##suffix_ = name_

// Same as MK_MOCK but with namespace
#define MK_MOCK_NAMESPACE(ns_, name_)                                          \
    decltype(ns_::name_) ns_##_##name_ = ns_::name_

// Same as MK_MOCK_NAMESPACE but with suffix
#define MK_MOCK_NAMESPACE_SUFFIX(ns_, name_, suffix_)                          \
    decltype(ns_::name_) ns_##_##name_##_##suffix_ = ns_::name_

/*
    The following allows you to skip generating too many templates when
    you're compiling for release, but at the same time allows to mock up
    the implementation when you compile for debugging.

    Note that, for this to work, you cannot use MK_MOCK_NAMESPACE.
*/
#ifdef MK_ENABLE_MOCK
#define MK_MOCK_TEMPLATE(_template_impl_) template < _template_impl_ >
#else
#define MK_MOCK_TEMPLATE(_template_impl_) /* nothing */
#endif

#endif
