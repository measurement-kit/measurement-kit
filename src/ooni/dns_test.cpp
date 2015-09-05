// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/dns/error.hpp>
#include <measurement_kit/ooni/dns_test.hpp>

namespace measurement_kit {
namespace ooni {

using namespace measurement_kit::dns;
using namespace measurement_kit::common;

void
DNSTest::query(QueryType query_type, QueryClass query_class,
               std::string query_name, std::string nameserver,
               std::function<void(Response)> cb)
{
    resolver = std::make_shared<Resolver>(Settings{
        {"nameserver", nameserver},
        {"attempts", "1"},
    }, &logger, libs);

    std::string nameserver_part;
    std::stringstream nameserver_ss(nameserver);
    std::getline(nameserver_ss, nameserver_part, ':');
    entry["resolver"].push_back(nameserver_part);
    std::getline(nameserver_ss, nameserver_part, ':');
    entry["resolver"].push_back(nameserver_part);

    resolver->query(
        query_class, query_type, query_name,
        [=](Error error, Response response) {
            logger.debug("dns_test: got response!");
            YAML::Node query_entry;
            if (query_type == QueryTypeId::A) {
              query_entry["query_type"] = "A";
              query_entry["query"] = "[Query('" + query_name +  ", 1, 1')]";
            }
            if (!error) {
                int idx = 0;
                for (auto result: response.get_results()) {
                    if (query_type == QueryTypeId::A) {
                        std::string rr;
                        rr = "<RR name=" + query_name + " ";
                        rr += "type=A class=IN ";
                        rr += "ttl=" + std::to_string(response.get_ttl()) + " ";
                        rr += "auth=False>, ";
                        rr += "<A address=" + result + " ";
                        rr += "ttl=" + std::to_string(response.get_ttl()) + ">";
                        query_entry["answers"][idx][0] = rr;
                    }
                    ++idx;
                }
            } else {
                query_entry["answers"][0] = NULL;
                query_entry["failure"] = error.as_ooni_error();
            }
            query_entry["rtt"] = response.get_rtt();
            // TODO add support for bytes received
            // query_entry["bytes"] = response.get_bytes();
            entry["queries"].push_back(query_entry);
            logger.debug("dns_test: callbacking");
            cb(response);
            logger.debug("dns_test: callback called");
    });
}

}}
