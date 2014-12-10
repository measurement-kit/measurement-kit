#include "ooni/dns_test.hpp"

using namespace ight::ooni::dns_test;

protocols::dns::Request query(QueryType query_type, QueryClass query_class,
                              std::string query_name, std::string nameserver,
                              std::function<void(protocols::dns::Response&&)>&& cb)
{
    resolver = protocols::dns::Resolver({
        {"nameserver", nameserver},
        {"attempts", "1"},
        });

    std::string nameserver_part;
    std::stringstream nameserver_ss(nameserver);
    std::getline(nameserver_ss, nameserver_part, ':');
    entry["resolver"].push_back(nameserver_part);
    std::getline(nameserver_ss, nameserver_part, ':');
    entry["resolver"].push_back(nameserver_part);

    YAML::Node query_entry;
    std::string query;
    if (query_type == QueryType::A) {
      query = "A";
      query_entry["query_type"] = "A";
      query_entry["query"] = "[Query('" + query_name +  ", 1, 1')]";
    } else {
      throw UnsupportedQueryType("Currently we only support A");
    }
    auto r = protocols::dns::Request(
        query, query_name, 
        [&](protocols::dns::Response&& response) {
        if (response.get_evdns_status() == DNS_ERR_NONE) {
        for (auto result: response.get_results()) {
        std::vector <std::string> answer;
        if (query_type == QueryType::A) {
        std::string rr;
        rr = "<RR name=" + query_name + " ";
        rr += "type=A class=IN ";
        rr += "ttl=" + std::to_string(response.get_rtt()) + " ";
        rr += "auth=False>, ";
        rr += "<A address=" + result + " ";
        rr += "ttl=" + std::to_string(response.get_rtt()) + ">";
        answer.push_back(rr);
        }
        query_entry["answers"].push_back(answer);
        }
        } else {
        query_entry["answers"].push_back(NULL);
        query_entry["failure"] = "generic_failure";
        }
        entry["queries"].push_back(query_entry);
        cb(std::move(response));
        },
        resolver.get_evdns_base(),
        options["nameserver"], libevent
        );
    return r;
}
