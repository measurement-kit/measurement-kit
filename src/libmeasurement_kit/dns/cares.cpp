// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <arpa/nameser.h>
#include <ares.h>
#include "../dns/query.hpp"

extern "C" {

static void
ares_cb(void *arg, int status, int timeouts, unsigned char *abuf, int alen) {
    mk::debug("%p %d %d [%d]", arg, status, timeouts, alen);
}

} // extern "C"
namespace mk {
namespace dns {
namespace cares {

void query(
        QueryClass dns_class,
        QueryType dns_type,
        std::string name,
        Callback<Error, Var<Message>> cb,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger) {

    int ares_code;

    if (!ares_library_initialized()) {
        if ((ares_code = ares_library_init(ARES_LIB_INIT_ALL)) != 0) {
            reactor->call_soon([=]() {
                cb(GenericError(), nullptr);
            });
            return;
        }
        // XXX: and now we need to decide whether to cleanup the library...
    }

    int mask = ARES_FLAG_NOCHECKRESP;
    ares_options options;
    memset(&options, 0, sizeof (options));

    ares_channel channel = nullptr;
    if ((ares_code = ares_init_options(&channel, &options, mask)) != 0) {
        reactor->call_soon([=]() {
            cb(GenericError(), nullptr);
        });
        return;
    }

    if ((ares_code = ares_set_servers_csv(channel, "8.8.8.8:53")) != 0) {
        reactor->call_soon([=]() {
            cb(GenericError(), nullptr);
        });
        return;
    }

    ares_query(channel, name.c_str(), ns_c_in, ns_t_a,
               ares_cb, nullptr);
}

} // namespace cares
} // namespace dns
} // namespace mk
