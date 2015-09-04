// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/dns/query.hpp>

#include <measurement_kit/common/libs.hpp>
#include <measurement_kit/common/pointer.hpp>
#include <measurement_kit/common/poller.hpp>

#include <functional>
#include <iosfwd>
#include <string>
#include <type_traits>

#include "src/dns/query_impl.hpp"

struct evdns_base;

namespace measurement_kit {
namespace common { class Logger; }
namespace dns {

using namespace measurement_kit::common;

Request::Request(std::string query, std::string address,
                 std::function<void(Response&&)>&& func,
                 Logger *lp, evdns_base *dnsb, Libs *libs)
{
    if (dnsb == nullptr) {
        dnsb = measurement_kit::get_global_evdns_base();
    }
    if (libs == nullptr) {
        libs = Libs::global();
    }
    cancelled = SharedPointer<bool>(new bool());
    *cancelled = false;
    RequestImpl::issue(query, address, std::move(func),
                       lp, dnsb, libs, cancelled);
}

void
Request::cancel(void)
{
    if (cancelled) {  // May not be set when we used the default constructor
        *cancelled = true;
    }
}

}}
