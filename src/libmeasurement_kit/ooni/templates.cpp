// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <event2/dns.h>

#include "src/libmeasurement_kit/ooni/error.hpp"
#include "src/libmeasurement_kit/ooni/templates_impl.hpp"
#include "src/libmeasurement_kit/net/emitter.hpp"
#include "src/libmeasurement_kit/ooni/utils.hpp"
#include "src/libmeasurement_kit/ooni/templates.hpp"

namespace mk {
namespace ooni {
namespace templates {

using namespace mk::report;

void dns_query(SharedPtr<Entry> entry, dns::QueryType query_type,
               dns::QueryClass query_class, std::string query_name,
               std::string nameserver, Callback<Error, SharedPtr<dns::Message>> cb,
               Settings options, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {

    std::string engine = options.get("dns/engine", std::string{"system"});
    bool not_system_engine = engine != "system";
    uint16_t resolver_port = 0;
    std::string resolver_hostname;

    SharedPtr<report::Entry> query_entry{new report::Entry};

    if (not_system_engine) {
        ErrorOr<net::Endpoint> maybe_epnt = net::parse_endpoint(nameserver, 53);
        if (!maybe_epnt) {
            reactor->call_soon([=]() { cb(maybe_epnt.as_error(), nullptr); });
            return;
        }
        resolver_port = maybe_epnt->port;
        resolver_hostname = maybe_epnt->hostname;
        options["dns/nameserver"] = resolver_hostname;
        options["dns/port"] = resolver_port;
        options["dns/attempts"] = 1;
        (*query_entry)["resolver_hostname"] = resolver_hostname;
        (*query_entry)["resolver_port"] = resolver_port;

    } else {
        if (nameserver != "") {
            logger->warn("Explicit nameserver ignored with 'system' DNS engine");
        }
        // For now this option is only supported by the system engine. Unless
        // user has already taken the decision whether to also resolve CNAME or
        // not, resolve the CNAME because generally we need that in OONI.
        if (options.count("dns/resolve_also_cname") == 0) {
            options["dns/resolve_also_cname"] = true;
        }
        // ooniprobe sets them to null when they are not available
        (*query_entry)["resolver_hostname"] = nullptr;
        (*query_entry)["resolver_port"] = nullptr;
    }

    dns::query(query_class, query_type, query_name,
               [=](Error error, SharedPtr<dns::Message> message) {
                   logger->debug("dns_test: got response!");
                   (*query_entry)["engine"] = engine;
                   (*query_entry)["failure"] = nullptr;
                   (*query_entry)["answers"] = Entry::array();
                   if (query_type == dns::MK_DNS_TYPE_A) {
                       (*query_entry)["query_type"] = "A";
                       (*query_entry)["hostname"] = query_name;
                   }
                   if (!error) {
                       for (auto answer : message->answers) {
                           nlohmann::json ttl; // = `null`
                           if (not_system_engine) {
                               ttl = answer.ttl;
                           }
                           if (answer.type == dns::MK_DNS_TYPE_A) {
                               (*query_entry)["answers"].push_back(
                                   {{"ttl", ttl},
                                    {"ipv4", answer.ipv4},
                                    {"answer_type", "A"}});
                           } else if (answer.type == dns::MK_DNS_TYPE_CNAME) {
                               (*query_entry)["answers"].push_back(
                                   {{"ttl", ttl},
                                    {"hostname", answer.hostname},
                                    {"answer_type", "CNAME"}});
                           }
                       }
                   } else {
                       (*query_entry)["failure"] = error.reason;
                   }
                   // TODO add support for bytes received
                   // (*query_entry)["bytes"] = response.get_bytes();
                   (*entry)["queries"].push_back(*query_entry);
                   logger->debug("dns_test: callbacking");
                   cb(error, message);
                   logger->debug("dns_test: callback called");
               },
               options, reactor);
}

void http_request(SharedPtr<Entry> entry, Settings settings, http::Headers headers,
                  std::string body, Callback<Error, SharedPtr<http::Response>> cb,
                  SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    http_request_impl<http::request>(entry, settings, headers, body,
                                     cb, reactor, logger);
}

void tcp_connect(Settings options,
        Callback<Error, SharedPtr<net::Transport>> cb,
        SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    ErrorOr<int> port = options["port"].as_noexcept<int>();
    if (!port) {
        cb(port.as_error(),
                SharedPtr<net::Transport>{
                        std::make_shared<net::Emitter>(reactor, logger)});
        return;
    }
    if (options["host"] == "") {
        cb(MissingRequiredHostError(),
                SharedPtr<net::Transport>{
                        std::make_shared<net::Emitter>(reactor, logger)});
        return;
    }
    net::connect(options["host"], *port, cb, options, reactor, logger);
}

} // namespace templates
} // namespace ooni
} // namespace mk
