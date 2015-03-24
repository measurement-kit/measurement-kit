#include <ight/ooni/dns_test.hpp>

using namespace ight::ooni::dns_test;
using namespace ight::protocols::dns;
using namespace ight::common;

void
DNSTest::query(QueryType query_type, QueryClass /*query_class*/,
               std::string query_name, std::string nameserver,
               std::function<void(ight::protocols::dns::Response&&)>&& cb)
{
    resolver = std::make_shared<Resolver>(Settings{
        {"nameserver", nameserver},
        {"attempts", "1"},
    }, libevent);

    std::string nameserver_part;
    std::stringstream nameserver_ss(nameserver);
    std::getline(nameserver_ss, nameserver_part, ':');
    entry["resolver"].push_back(nameserver_part);
    std::getline(nameserver_ss, nameserver_part, ':');
    entry["resolver"].push_back(nameserver_part);

    std::string query;
    if (query_type == QueryType::A) {
        query = "A";
    } else {
        throw UnsupportedQueryType("Currently we only support A");
    }
    resolver->request(
        query, query_name, 
        [=](ight::protocols::dns::Response&& response) {
            logger->debug("dns_test: got response!");
            YAML::Node query_entry;
            if (query_type == QueryType::A) {
              query_entry["query_type"] = "A";
              query_entry["query"] = "[Query('" + query_name +  ", 1, 1')]";
            }
            if (response.get_evdns_status() == DNS_ERR_NONE) {
                int idx = 0;
                for (auto result: response.get_results()) {
                    if (query_type == QueryType::A) {
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
                query_entry["failure"] = response.get_failure();
            }
            query_entry["rtt"] = response.get_rtt();
            // TODO add support for bytes received
            // query_entry["bytes"] = response.get_bytes();
            entry["queries"].push_back(query_entry);
            logger->debug("dns_test: callbacking");
            cb(std::move(response));
            logger->debug("dns_test: callback called");
    });
}
