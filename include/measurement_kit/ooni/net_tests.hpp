// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_NET_TESTS_HPP
#define MEASUREMENT_KIT_OONI_NET_TESTS_HPP

// Low level functions that implement individual OONI tests by
// running them towards a single specific input

#include <measurement_kit/common.hpp>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {

/*
 ____  _   _ ____
|  _ \| \ | / ___|
| | | |  \| \___ \
| |_| | |\  |___) |
|____/|_| \_|____/

    Tests and templates using the DNS.
*/

void dns_injection(std::string name_server, std::string input,
                   Callback<Error, Var<report::Entry>> callback,
                   Settings settings = {}, Var<Reactor> = Reactor::global(),
                   Var<Logger> = Logger::global());

void dns_template(Var<report::Entry>, std::string name_server, dns::QueryType,
                  dns::QueryClass, std::string query_name,
                  Callback<Error, dns::Message>, Settings = {},
                  Var<Reactor> = Reactor::global(),
                  Var<Logger> = Logger::global());

} // namespace ooni
} // namespace mk
#endif
