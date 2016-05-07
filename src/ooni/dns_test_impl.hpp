// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_DNS_TEST_HPP
#define SRC_OONI_DNS_TEST_HPP

#include <measurement_kit/common/var.hpp>

#include <measurement_kit/dns.hpp>
#include "src/ooni/ooni_test_impl.hpp"

using json = nlohmann::json;

namespace mk {
namespace ooni {

class DNSTestImpl : public ooni::OoniTestImpl {
    using ooni::OoniTestImpl::OoniTestImpl;

  public:
    DNSTestImpl(std::string input_filepath_, Settings options_)
        : ooni::OoniTestImpl(input_filepath_, options_) {
        test_name = "dns_test";
        test_version = "0.0.1";
    };

    void query(dns::QueryType query_type, dns::QueryClass query_class,
               std::string query_name, std::string nameserver,
               std::function<void(dns::Message)> cb) {

        int resolver_port;
        std::string resolver_hostname, nameserver_part;
        std::stringstream nameserver_ss(nameserver);

        std::getline(nameserver_ss, nameserver_part, ':');
        resolver_hostname = nameserver_part;

        std::getline(nameserver_ss, nameserver_part, ':');
        resolver_port = std::stoi(nameserver_part, nullptr);

        dns::query(
            query_class, query_type, query_name,
            [=](Error error, dns::Message message) {
                logger.debug("dns_test: got response!");
                json query_entry;
                query_entry["resolver_hostname"] = resolver_hostname;
                query_entry["resolver_port"] = resolver_port;
                query_entry["failure"] = nullptr;
                query_entry["answers"] = {};
                if (query_type == dns::QueryTypeId::A) {
                    query_entry["query_type"] = "A";
                    query_entry["hostname"] = query_name;
                }
                if (!error) {
                    for (auto answer : message.answers) {
                        if (query_type == dns::QueryTypeId::A) {
                        query_entry["answers"].push_back({
                                {"ttl", answer.ttl},
                                {"ipv4", answer.ipv4},
                                {"answer_type", "A"}
                            });
                        }
                    }
                } else {
                    query_entry["failure"] = error.as_ooni_error();
                }
                // TODO add support for bytes received
                // query_entry["bytes"] = response.get_bytes();
                entry["queries"].push_back(query_entry);
                logger.debug("dns_test: callbacking");
                cb(message);
                logger.debug("dns_test: callback called");
            }, Settings{
                {"dns/nameserver", nameserver}, {"dns/attempts", "1"},
            }, poller);
    }
};

} // namespace ooni
} // namespace mk
#endif
