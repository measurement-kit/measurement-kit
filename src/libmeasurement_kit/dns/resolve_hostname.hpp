// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_RESOLVE_HOSTNAME_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_RESOLVE_HOSTNAME_HPP

#include <measurement_kit/common.hpp>
#include "src/libmeasurement_kit/dns/query.hpp"

namespace mk {
namespace dns {

class ResolveHostnameResult {
  public:
    bool inet_pton_ipv4 = false;
    bool inet_pton_ipv6 = false;

    Error ipv4_err;
    dns::Message ipv4_reply;
    Error ipv6_err;
    dns::Message ipv6_reply;

    std::vector<std::string> addresses;
};

void resolve_hostname(std::string hostname,
        Callback<ResolveHostnameResult> cb,
        Settings settings = {},
        SharedPtr<Reactor> reactor = Reactor::global(),
        SharedPtr<Logger> logger = Logger::global());

} // namespace dns
} // namespace mk
#endif
