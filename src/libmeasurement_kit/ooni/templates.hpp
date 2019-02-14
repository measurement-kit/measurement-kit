// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_TEMPLATES_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_TEMPLATES_HPP

#include "src/libmeasurement_kit/net/connect.hpp"
#include "src/libmeasurement_kit/net/transport.hpp"
#include "src/libmeasurement_kit/dns/query.hpp"
#include "src/libmeasurement_kit/http/http.hpp"

namespace mk {
namespace ooni {
namespace templates {

void dns_query(SharedPtr<nlohmann::json> entry, dns::QueryType, dns::QueryClass,
               std::string query_name, std::string nameserver,
               Callback<Error, SharedPtr<dns::Message>>, Settings,
               SharedPtr<Reactor>, SharedPtr<Logger>);

void http_request(SharedPtr<nlohmann::json> entry, Settings settings, http::Headers headers,
                  std::string body, Callback<Error, SharedPtr<http::Response>> cb,
                  SharedPtr<Reactor> reactor, SharedPtr<Logger> logger);

void tcp_connect(Settings options, Callback<Error, SharedPtr<net::Transport>> cb,
                 SharedPtr<Reactor> reactor, SharedPtr<Logger> logger);

} // namespace templates
} // namespace mk
} // namespace ooni
#endif
