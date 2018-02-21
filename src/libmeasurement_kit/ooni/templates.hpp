// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_TEMPLATES_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_TEMPLATES_HPP

#include <measurement_kit/dns.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/net.hpp>

#include "src/libmeasurement_kit/report/entry.hpp"

namespace mk {
namespace ooni {
namespace templates {

void dns_query(SharedPtr<report::Entry> entry, dns::QueryType, dns::QueryClass,
               std::string query_name, std::string nameserver,
               Callback<Error, SharedPtr<dns::Message>>, Settings = {},
               SharedPtr<Reactor> = Reactor::global(),
               SharedPtr<Logger> = Logger::global());

void http_request(SharedPtr<report::Entry> entry, Settings settings, http::Headers headers,
                  std::string body, Callback<Error, SharedPtr<http::Response>> cb,
                  SharedPtr<Reactor> reactor = Reactor::global(),
                  SharedPtr<Logger> logger = Logger::global());

void tcp_connect(Settings options, Callback<Error, SharedPtr<net::Transport>> cb,
                 SharedPtr<Reactor> reactor = Reactor::global(),
                 SharedPtr<Logger> logger = Logger::global());

} // namespace templates
} // namespace mk
} // namespace ooni
#endif
