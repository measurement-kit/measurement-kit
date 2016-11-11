// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../dns/query.hpp"

#include <arpa/nameser.h> // XXX: ditto
#include <netdb.h>        // XXX: how portable is this header?

#include <ares.h>
#include <cassert>

namespace mk {
namespace dns {
namespace cares {

// TODO: mock the called APIs and write regress tests
void query(QueryClass dns_class, QueryType dns_type, std::string name,
           Callback<Error, Var<Message>> cb, Settings settings,
           Var<Reactor> reactor, Var<Logger> logger) {

#define SCHED_ERROR(e_)                                                        \
    do {                                                                       \
        reactor->call_soon([=]() { cb(e_, nullptr); });                        \
    } while (0)

    int ares_code;

    // TODO: add error types for common errors and make sure they wrap
    // underlying errors coming from libcares

    if (!ares_library_initialized()) {
        if ((ares_code = ares_library_init(ARES_LIB_INIT_ALL)) != 0) {
            SCHED_ERROR(GenericError());
            return;
        }
        // XXX: and now we need to decide whether to cleanup the library...
        // TODO: perhaps we can use atexit()?
    }

    int mask = ARES_FLAG_NOCHECKRESP;
    ares_options options;
    memset(&options, 0, sizeof(options));
    ares_channel channel = nullptr;
    if ((ares_code = ares_init_options(&channel, &options, mask)) != 0) {
        SCHED_ERROR(GenericError());
        return;
    }

    std::string nameserver = settings.get("dns/nameserver", std::string{});
    if (nameserver == "") {
        // This engine does not work unless you pass it a nameserver
        SCHED_ERROR(GenericError());
        return;
    }

    if ((ares_code = ares_set_servers_csv(channel, nameserver.c_str())) != 0) {
        SCHED_ERROR(GenericError());
        return;
    }

    if (dns_class != "IN") {
        // Note: this engine only works for the "IN" domain
        SCHED_ERROR(GenericError());
        return;
    }

    unsigned char *buf = nullptr;
    int buflen = 0;
    int ares_class = 0;
    int ares_type = ns_c_in;
    if (dns_type == "A") {
        ares_type = ns_t_al;
    } else {
        SCHED_ERROR(GenericError());
        return;
    }

    ares_code = ares_create_query(name.c_str(), ares_class, ares_type,
                                  0, // FIXME: we should pass here a specific ID
                                  1, // XXX: okay to pass nonzero recursion
                                  &buf, &buflen,
                                  0 // Note: this is for EDNS
                                  );
    if (ares_code != 0) {
        SCHED_ERROR(GenericError());
        return;
    }
    assert(buf != nullptr);
    assert(buflen > 0);

    // XXX: Here I've fucking hardcoded stuff
    // XXX: Also, I'd rather log the request message
    ares_query(channel, name.c_str(), ns_c_in, ns_t_a, ares_cb, nullptr);

    mk::debug("here...");

    // FIXME: the way in which I do this sucks from many points of view...
    ares_socket_t sockfd = -1;
    ares_getsock(channel, &sockfd, 1); /* XXX */
    if (sockfd == -1) {
        reactor->call_soon([=]() { cb(GenericError(), nullptr); });
        return;
    }

    mk::debug("the sock is: %d", sockfd);

    reactor->pollfd(sockfd, MK_POLLIN,
                    [=](Error err, short flags) {
                        mk::debug("cb: %s %d", err.explain().c_str(), flags);
                        if (err) {
                            cb(err, nullptr);
                            return;
                        }
                        if ((flags & MK_POLLIN) == 0) {
                            cb(GenericError(), nullptr);
                            return;
                        }
                        // ares_process_fd(channel, sockfd, ARES_SOCKET_BAD);
                        unsigned char response[8192];
                        ssize_t retval =
                            recv(sockfd, response, sizeof(response), 0);
                        if (retval < 0) {
                            cb(GenericError(), nullptr);
                            return;
                        }
                        mk::debug("we've read: %d", retval);
                        hostent *host = nullptr;
                        ares_addrttl ttls;
                        memset(&ttls, 0, sizeof(ttls));
                        int nttls = 0;
                        if (ares_parse_a_reply(response, retval, &host, &ttls,
                                               &nttls) != 0) {
                            cb(GenericError(), nullptr);
                            return;
                        }
                        assert(nttls >= 0);
                        mk::debug("parsing okay");
                        mk::debug("host name: %d", host->h_length);
                        for (char **p = host->h_addr_list; *p != nullptr; ++p) {
                            in_addr addr;
                            memcpy(&addr, *p, 4);
                            mk::debug("host name: %s", inet_ntoa(addr));
                        }
                    },
                    3.0);
}

} // namespace cares
} // namespace dns
} // namespace mk
