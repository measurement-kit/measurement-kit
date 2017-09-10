// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/common/locked.hpp"
#include "private/libevent/reactor.hpp"
#include <iostream>

namespace mk {

Reactor::Impl::~Impl() {}

Reactor::Reactor() : impl_{std::make_shared<libevent::Reactor<>>()} {}

/*static*/ Reactor Reactor::global() {
    return locked_global([]() {
        static Reactor singleton;
        return singleton;
    });
}

} // namespace mk
