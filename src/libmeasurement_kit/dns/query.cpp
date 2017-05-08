// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../libevent/dns.hpp"
#include "../dns/system_resolver.hpp"

namespace mk {
namespace dns {

void query(
        QueryClass dns_class,
        QueryType dns_type,
        std::string name,
        Callback<Error, Var<Message>> cb,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger) {
    NameServers ns;
    if (settings.count("dns/nameserver") != 0) {
        ns.push_back(settings.at("dns/nameserver"));
    }
    query_with_nameservers(ns, dns_class, dns_type, name, settings,
                           reactor, logger, cb);
}

void query_with_nameservers(
        NameServers name_servers,
        QueryClass dns_class,
        QueryType dns_type,
        std::string name_to_query,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger,
        Callback<Error, Var<Message>> cb) {
    std::string engine = settings.get("dns/engine", std::string("system"));
    logger->log(MK_LOG_DEBUG2, "dns: engine: %s", engine.c_str());
    if (engine == "libevent") {
        libevent::query(name_servers, dns_class, dns_type, name_to_query,
                        settings, reactor, logger, cb);
    } else if (engine == "system") {
        system_resolver(name_servers, dns_class, dns_type, name_to_query,
                        settings, reactor, logger, cb);
    } else {
        reactor->call_soon([cb]() { cb(InvalidDnsEngine(), nullptr); });
    }
}

void resolve_hostname(std::string hostname, Callback<ResolveHostnameResult> cb,
                      Settings settings, Var<Reactor> reactor,
                      Var<Logger> logger) {

    logger->debug("resolve_hostname: %s", hostname.c_str());

    sockaddr_storage storage;
    Var<ResolveHostnameResult> result(new ResolveHostnameResult);

    // If address is a valid IPv4 address, connect directly
    memset(&storage, 0, sizeof storage);
    if (inet_pton(PF_INET, hostname.c_str(), &storage) == 1) {
        logger->debug("resolve_hostname: is valid ipv4");
        result->addresses.push_back(hostname);
        result->inet_pton_ipv4 = true;
        cb(*result);
        return;
    }

    // If address is a valid IPv6 address, connect directly
    memset(&storage, 0, sizeof storage);
    if (inet_pton(PF_INET6, hostname.c_str(), &storage) == 1) {
        logger->debug("resolve_hostname: is valid ipv6");
        result->addresses.push_back(hostname);
        result->inet_pton_ipv6 = true;
        cb(*result);
        return;
    }

    logger->debug("resolve_hostname: ipv4...");

    dns::query("IN", "A", hostname,
               [=](Error err, Var<dns::Message> resp) {
                   logger->debug("resolve_hostname: ipv4... done");
                   result->ipv4_err = err;
                   if (!err) {
                       result->ipv4_reply = *resp;
                       for (dns::Answer answer : resp->answers) {
                           result->addresses.push_back(answer.ipv4);
                       }
                   }
                   logger->debug("resolve_hostname: ipv6...");
                   dns::query(
                       "IN", "AAAA", hostname,
                       [=](Error err, Var<dns::Message> resp) {
                           logger->debug("resolve_hostname: ipv6... done");
                           result->ipv6_err = err;
                           if (!err) {
                               result->ipv6_reply = *resp;
                               for (dns::Answer answer : resp->answers) {
                                   result->addresses.push_back(answer.ipv6);
                               }
                           }
                           cb(*result);
                       },
                       settings, reactor, logger);
               },
               settings, reactor, logger);
}

} // namespace dns
} // namespace mk
