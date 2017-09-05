// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "private/common/fcompose.hpp"
#include "private/common/utils.hpp"
#include "private/ooni/utils.hpp"
#include "private/ooni/constants.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

static void dns_many(Error error,
                     Var<Entry> entry,
                     std::map<std::string, std::string> fb_service_hostnames,
                     Settings options,
                     Var<Reactor> reactor,
                     Var<Logger> logger,
                     Callback<Error,
                              Var<Entry>,
                              std::map<std::string, std::vector<std::string>>,
                              Settings,
                              Var<Reactor>,
                              Var<Logger>> cb
                    ) {
    Var<std::map<std::string, std::vector<std::string>>>
        fb_service_ips(new std::map<std::string, std::vector<std::string>>(
            {{"stun", {}},
             {"b_api", {}},
             {"b_graph", {}},
             {"edge", {}},
             {"external_cdn", {}},
             {"scontent_cdn", {}},
             {"star", {}}}
        ));

    if (error) {
        cb(error, entry, *fb_service_ips, options, reactor, logger);
        return;
    }

    // if we find any inconsistent DNS later, switch this to true
    (*entry)["facebook_dns_blocking"] = false;

    int names_count = fb_service_hostnames.size();
    Var<int> names_tested(new int(0));

    if (names_count == *names_tested) {
        cb(NoError(), entry, *fb_service_ips, options, reactor, logger);
        return;
    }

    auto dns_cb = [=](std::string service, std::string hostname) {
        return [=](Error err, Var<dns::Message> message) {
            if (!!err) {
                logger->info("fb_messenger: dns error for %s, %s",
                    service.c_str(), hostname.c_str());
            } else {
                for (auto answer : message->answers) {
                    if ((answer.ipv4 != "") || (answer.hostname != "")) {
                        std::string asn_p = options.get("geoip_asn_path", std::string{});
                        auto geoip = GeoipCache::thread_local_instance()->get(asn_p);
                        ErrorOr<std::string> asn = geoip->resolve_asn(answer.ipv4);
                        if (asn && asn.as_value() != "AS0") {
                            logger->info("%s ipv4: %s, %s",
                                hostname.c_str(), answer.ipv4.c_str(),
                                asn.as_value().c_str());
                            // if consistent, add to list for tcp connect later
                            if (asn.as_value() == "AS32934") {
                                (*fb_service_ips)[service].push_back(answer.ipv4);
                            } else {
                                (*entry)["facebook_dns_blocking"] = true;
                            }
                        }
                    }
                }
                // if any consistent IPs, consistent = true
                (*entry)["facebook_" + service + "_dns_consistent"] =
                    !(*fb_service_ips)[service].empty();
            }
            *names_tested += 1;
            if (names_count == *names_tested) {
                cb(NoError(), entry, *fb_service_ips, options, reactor, logger);
                return;
            }
        };
    };

    for (auto const& service_and_hostname : fb_service_hostnames) {
        std::string service = service_and_hostname.first;
        std::string hostname = service_and_hostname.second;
        templates::dns_query(entry, "A", "IN", hostname, "",
                             dns_cb(service, hostname), options, reactor,
                             logger);
    }
    return;
}

static void tcp_many(Error error,
                     Var<Entry> entry,
                     std::map<std::string, std::vector<std::string>> fb_service_ips,
                     Settings options,
                     Var<Reactor> reactor,
                     Var<Logger> logger,
                     Callback<Var<Entry>> cb
                    ) {
    logger->info("starting tcp_many");
    if (error) {
        cb(entry);
        return;
    }

    // if we find any blocked TCP later, switch this to true
    (*entry)["facebook_dns_blocking"] = false;

    int ips_count = 0;
    for (auto const& service_and_ips : fb_service_ips) {
        // skipping stun for now
        if (service_and_ips.first == "stun") {
            continue;
        }
        for (auto const& ip : service_and_ips.second) {
            ips_count += 1;
        }
    }
    if (ips_count == 0) {
        cb(entry);
        return;
    }
    Var<int> ips_tested(new int(0));

    auto tcp_cb = [=](std::string service, std::string ip, int port) {
        return [=](Error err, Var<net::Transport> txp) {
            bool close_txp = true; // if connected, we must disconnect
            if (!!err) {
                logger->info("tcp failure to %s at %s:%d", service.c_str(),
                    ip.c_str(), port);
                (*entry)["facebook_" + service + "_blocking"] = true;
                (*entry)["facebook_dns_blocking"] = true;
                close_txp = false;
            } else {
                logger->info("tcp success to %s at %s:%d", service.c_str(),
                    ip.c_str(), port);
                (*entry)["facebook_" + service + "_blocking"] = false;
            }
            *ips_tested += 1;
            if (ips_count == *ips_tested) {
                if (close_txp == true) {
                    txp->close([=] {
                        cb(entry);
                        return;
                    });
                } else {
                    cb(entry);
                    return;
                }
            } else {
                if (close_txp == true) {
                    // XXX optimistic closure
                    txp->close([=] {});
                }
            }
        };
    };

    for (auto const& service_and_ips : fb_service_ips) {
        std::string service = service_and_ips.first;
        // skipping stun for now
        if (service == "stun") {
            continue;
        }
        for (auto const& ip : service_and_ips.second) {
            Settings tcp_options;
            tcp_options["host"] = ip;
            int port = 443; //XXX hardcoded
            tcp_options["port"] = port;
            tcp_options["net/timeout"] = 10.0; //XXX hardcoded
            templates::tcp_connect(tcp_options, tcp_cb(service, ip, port),
                                   reactor, logger);
        }
    }
    return;
}

void facebook_messenger(Settings options,
              Callback<Var<report::Entry>> callback,
              Var<Reactor> reactor, Var<Logger> logger) {
    logger->info("starting facebook_messenger");
    Var<Entry> entry(new Entry);

    std::map<std::string, std::string>
        FB_SERVICE_HOSTNAMES = {
            {"stun", "stun.fbsbx.com"},
            {"b_api", "b-api.facebook.com"},
            {"b_graph", "b-graph.facebook.com"},
            {"edge", "edge-mqtt.facebook.com"},
            {"external_cdn", "external.xx.fbcdn.net"},
            {"scontent_cdn", "scontent.xx.fbcdn.net"},
            {"star", "star.c10r.facebook.com"}};

    options["geoip_asn_path"] = "GeoIPASNum.dat";

    mk::fcompose(
                 mk::fcompose_policy_async(),
                 dns_many,
                 tcp_many
                )(NoError(),
                  entry,
                  FB_SERVICE_HOSTNAMES,
                  options,
                  reactor,
                  logger,
                  callback
                 );

    return;
}

} // namespace ooni
} // namespace mk
