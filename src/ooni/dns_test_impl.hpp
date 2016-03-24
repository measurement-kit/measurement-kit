// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_DNS_TEST_HPP
#define SRC_OONI_DNS_TEST_HPP

#include <measurement_kit/common/var.hpp>

#include <measurement_kit/dns.hpp>
#include "src/ooni/ooni_test_impl.hpp"

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

        std::string nameserver_part;
        std::stringstream nameserver_ss(nameserver);
        std::getline(nameserver_ss, nameserver_part, ':');
        entry["resolver"].push_back(nameserver_part);
        std::getline(nameserver_ss, nameserver_part, ':');
        entry["resolver"].push_back(nameserver_part);

        dns::query(
            query_class, query_type, query_name,
            [=](Error error, dns::Message message) {
                logger.debug("dns_test: got response!");
                YAML::Node query_entry;
                if (query_type == dns::QueryTypeId::A) {
                    query_entry["query_type"] = "A";
                    query_entry["query"] =
                        "[Query('" + query_name + ", 1, 1')]";
                }
                if (!error) {
                    int idx = 0;
                    for (auto answer : message.answers) {
                        if (query_type == dns::QueryTypeId::A) {
                            std::string rr;
                            rr = "<RR name=" + query_name + " ";
                            rr += "type=A class=IN ";
                            rr += "ttl=" + std::to_string(answer.ttl) +
                                  " ";
                            rr += "auth=False>, ";
                            rr += "<A address=" + answer.ipv4 + " ";
                            rr += "ttl=" + std::to_string(answer.ttl) +
                                  ">";
                            query_entry["answers"][idx][0] = rr;
                        }
                        ++idx;
                    }
                } else {
                    query_entry["answers"][0] = NULL;
                    query_entry["failure"] = error.as_ooni_error();
                }
                query_entry["rtt"] = message.rtt;
                // TODO add support for bytes received
                // query_entry["bytes"] = response.get_bytes();
                entry["queries"].push_back(query_entry);
                logger.debug("dns_test: callbacking");
                cb(message);
                logger.debug("dns_test: callback called");
            }, Settings{
                {"nameserver", nameserver}, {"attempts", "1"},
            }, poller);
    }
};

} // namespace ooni
} // namespace mk
#endif
