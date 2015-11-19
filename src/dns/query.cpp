// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/dns/defines.hpp>
#include <measurement_kit/dns/query.hpp>

#include <measurement_kit/common/libs.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/var.hpp>

#include <functional>
#include <iosfwd>
#include <string>
#include <type_traits>

#include "src/dns/query_impl.hpp"

struct evdns_base;

namespace measurement_kit {
namespace common {
class Logger;
}
namespace dns {

using namespace measurement_kit::common;

Query::Query(QueryClass dns_class, QueryType dns_type, std::string name,
             std::function<void(Error, Response)> func, Logger *lp,
             evdns_base *dnsb, Libs *libs) {
    if (dnsb == nullptr) {
        dnsb = measurement_kit::get_global_evdns_base();
    }
    if (libs == nullptr) {
        libs = Libs::global();
    }
    cancelled = Var<bool>(new bool());
    *cancelled = false;
    QueryImpl::issue(dns_class, dns_type, name, func, lp,
                     dnsb, libs, cancelled);
}

void Query::cancel(void) {
    if (cancelled) { // May not be set when we used the default constructor
        *cancelled = true;
    }
}

} // namespace dns
} // namespace measurement_kit
