// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <event2/dns.h>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/ooni.hpp>
#include <measurement_kit/report.hpp>
#include <sstream>

namespace mk {
namespace ooni {
namespace templates {

using namespace mk::report;

void dns_query(Var<Entry> entry, dns::QueryType query_type,
               dns::QueryClass query_class, std::string query_name,
               std::string nameserver, Callback<Error, dns::Message> cb,
               Settings options, Var<Reactor> reactor, Var<Logger> logger) {

    int resolver_port;
    std::string resolver_hostname, nameserver_part;
    std::stringstream nameserver_ss(nameserver);

    std::getline(nameserver_ss, nameserver_part, ':');
    resolver_hostname = nameserver_part;

    std::getline(nameserver_ss, nameserver_part, ':');
    resolver_port = std::stoi(nameserver_part, nullptr);

    options["dns/nameserver"] = nameserver;
    options["dns/attempts"] = 1;

    dns::query(query_class, query_type, query_name,
               [=](Error error, dns::Message message) {
                   logger->debug("dns_test: got response!");
                   Entry query_entry;
                   query_entry["resolver_hostname"] = resolver_hostname;
                   query_entry["resolver_port"] = resolver_port;
                   query_entry["failure"] = nullptr;
                   query_entry["answers"] = Entry::array();
                   if (query_type == dns::QueryTypeId::A) {
                       query_entry["query_type"] = "A";
                       query_entry["hostname"] = query_name;
                   }
                   if (!error) {
                       for (auto answer : message.answers) {
                           if (query_type == dns::QueryTypeId::A) {
                               query_entry["answers"].push_back(
                                   {{"ttl", answer.ttl},
                                    {"ipv4", answer.ipv4},
                                    {"answer_type", "A"}});
                           }
                       }
                   } else {
                       query_entry["failure"] = error.as_ooni_error();
                   }
                   // TODO add support for bytes received
                   // query_entry["bytes"] = response.get_bytes();
                   (*entry)["queries"].push_back(query_entry);
                   logger->debug("dns_test: callbacking");
                   cb(error, message);
                   logger->debug("dns_test: callback called");
               },
               options, reactor);
}

void http_request(Var<Entry> entry, Settings settings, http::Headers headers,
                  std::string body, Callback<Error, Var<http::Response>> cb,
                  Var<Reactor> reactor, Var<Logger> logger) {

    (*entry)["agent"] = "agent";
    (*entry)["socksproxy"] = nullptr;

    if (settings.find("http/method") == settings.end()) {
        settings["http/method"] = "GET";
    }

    http::request(
        settings, headers, body,
        [=](Error error, Var<http::Response> response) {
            Entry rr;
            rr["request"]["headers"] =
                std::map<std::string, std::string>(headers);
            rr["request"]["body"] = body;
            rr["request"]["url"] = settings.at("http/url").c_str();
            rr["request"]["method"] = settings.at("http/method").c_str();

            // XXX we should probably update OONI data format to remove this.
            rr["method"] = settings.at("http/method").c_str();

            if (!error) {
                rr["response"]["headers"] =
                    std::map<std::string, std::string>(response->headers);
                rr["response"]["body"] = response->body;
                rr["response"]["response_line"] = response->response_line;
                rr["response"]["code"] = response->status_code;
                rr["failure"] = nullptr;
            } else {
                rr["failure"] = error.as_ooni_error();
            }

            (*entry)["requests"].push_back(rr);
            cb(error, response);
        },
        reactor, logger);
}

void tcp_connect(Settings options, Callback<Error, Var<net::Transport>> cb,
                 Var<Reactor> reactor, Var<Logger> logger) {
    ErrorOr<int> port = options["port"].as_noexcept<int>();
    if (!port) {
        cb(port.as_error(), nullptr);
        return;
    }
    if (options["host"] == "") {
        cb(MissingRequiredHostError(), nullptr);
        return;
    }
    net::connect(options["host"], *port, cb, options, logger, reactor);
}

} // namespace templates
} // namespace ooni
} // namespace mk
