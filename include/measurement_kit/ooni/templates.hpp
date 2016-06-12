// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_TEMPLATES_HPP
#define MEASUREMENT_KIT_OONI_TEMPLATES_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {
namespace templates {

void dns_query(Var<Entry> entry, dns::QueryType, dns::QueryClass,
               std::string query_name, std::string nameserver,
               Callback<Error, dns::Message>, Settings = {},
               Var<Reactor> = Reactor::global(),
               Var<Logger> = Logger::global());

void http_request(Var<Entry> entry, Settings settings, http::Headers headers,
                  std::string body, Callback<Error, Var<http::Response>> cb,
                  Var<Reactor> reactor = Reactor::global(),
                  Var<Logger> logger = Logger::global());

void tcp_connect(Settings options, Callback<Error, Var<net::Transport>> cb,
                 Var<Reactor> reactor = Reactor::global(),
                 Var<Logger> logger = Logger::global());

} // namespace templates
} // namespace mk
} // namespace ooni
#endif
