// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/dns/query.hpp>
#include <measurement_kit/dns/response.hpp>
#include <measurement_kit/dns/resolver.hpp>

#include <measurement_kit/common/libs.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/pointer.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/utils.hpp>

#include <event2/dns.h>

#include <cassert>
#include <functional>
#include <iosfwd>
#include <map>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <netinet/in.h>
#include <sys/socket.h>

#include "src/dns/query_impl.hpp"

struct evdns_base;

namespace measurement_kit {
namespace dns {

using namespace measurement_kit::common;

void
Resolver::cleanup(void)
{
    if (base != nullptr) {
        //
        // Note: `1` means that pending requests are notified that
        // this evdns_base is being closed, i.e., their callback is
        // called with error DNS_ERROR_SHUTDOWN.
        //
        // We need to call evdns_base_free() like this because
        // this guarantees that request's callback is always invoked
        // so RequestImpl:s are always freed (see request()).
        //
        libs->evdns_base_free(base, 1);
        base = nullptr;  // Idempotent
    }
}

evdns_base *
Resolver::get_evdns_base(void)
{
    if (base != nullptr) {  // Idempotent
        return base;
    }

    //
    // Note: in case of error, the object state is reset
    // like nothing has happened.
    //

    auto evb = poller->get_event_base();
    if (settings.find("nameserver") != settings.end()) {
        if ((base = libs->evdns_base_new(evb, 0)) == nullptr) {
            throw std::bad_alloc();
        }
        if (libs->evdns_base_nameserver_ip_add(base,
                settings["nameserver"].c_str()) != 0) {
            cleanup();
            throw std::runtime_error("Cannot set server address");
        }
    } else if ((base = libs->evdns_base_new(evb, 1)) == nullptr) {
        throw std::bad_alloc();
    }

    if (settings.find("attempts") != settings.end() &&
            libs->evdns_base_set_option(base,
            "attempts", settings["attempts"].c_str()) != 0) {
        cleanup();
        throw std::runtime_error("Cannot set 'attempts' option");
    }
    if (settings.find("timeout") != settings.end() &&
            libs->evdns_base_set_option(
            base, "timeout", settings["timeout"].c_str()) != 0) {
        cleanup();
        throw std::runtime_error("Cannot set 'timeout' option");
    }

    // By default we don't randomize the query's case
    std::string randomiz{"0"};
    if (settings.find("randomize_case") != settings.end()) {
        randomiz = settings["randomize_case"];
    }
    if (libs->evdns_base_set_option(base, "randomize-case",
            randomiz.c_str()) != 0) {
        cleanup();
        throw std::runtime_error("Cannot set 'randomize-case' option");
    }

    return base;
}

void
Resolver::request(std::string query, std::string address,
        std::function<void(Response&&)>&& func)
{
    //
    // Note: RequestImpl implements the autodelete behavior, meaning that
    // it shall delete itself once its callback is called. The callback
    // should always be called, either because of a successful response,
    // or because of an error, or because the resolver is destroyed (this
    // is guaranteed by the destructor's impl).
    //
    auto cancelled = SharedPointer<bool>(new bool());
    *cancelled = false;
    RequestImpl::issue(query, address, std::move(func), logger,
                       get_evdns_base(), libs, cancelled);
}

}}
