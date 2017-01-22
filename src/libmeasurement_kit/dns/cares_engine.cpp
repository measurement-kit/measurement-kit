// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../dns/cares_engine_impl.hpp"

namespace mk {
namespace dns {

void cares_engine_query(QueryClass dns_class, QueryType dns_type,
                        std::string name, Callback<Error, Var<Message>> cb,
                        Settings settings, Var<Reactor> reactor,
                        Var<Logger> logger) {
    /*
     * This `call_soon()` here is to _schedule_ the execution of the
     * requested action and this make sure the callback is called _after_
     * the `sendrecv_impl()` function returns. I am really pissed off
     * when a callback is called before the function with which you did
     * registered the callback returns: it increases complexity.
     */
    reactor->call_soon([=]() {
        cares_engine_query_impl(dns_class, dns_type, name, cb, settings,
                                reactor, logger, 1);
    });
}

} // namespace dns
} // namespace mk
