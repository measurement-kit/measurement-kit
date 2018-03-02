// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ooni/constants.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"
#include "src/libmeasurement_kit/ooni/utils.hpp"
#include "src/libmeasurement_kit/common/fcompose.hpp"
#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/ooni/templates.hpp"

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

static const std::string FB_ASN = "AS32934";

static const std::map<std::string, std::string> &FB_SERVICE_HOSTNAMES = {
      {"stun", "stun.fbsbx.com"},
      {"b_api", "b-api.facebook.com"},
      {"b_graph", "b-graph.facebook.com"},
      {"edge", "edge-mqtt.facebook.com"},
      {"external_cdn", "external.xx.fbcdn.net"},
      {"scontent_cdn", "scontent.xx.fbcdn.net"},
      {"star", "star.c10r.facebook.com"}};

static bool ip_in_fb_asn(Settings options, std::string ip) {
    std::string asn_p = options.get("geoip_asn_path", std::string{});
    auto geoip = GeoipCache::thread_local_instance()->get(asn_p);
    ErrorOr<std::string> asn = geoip->resolve_asn(ip);
    if (!!asn && asn.as_value() != "AS0") {
        return asn.as_value().c_str() == FB_ASN;
    }
    return false;
}

static void
dns_many(Error error, SharedPtr<Entry> entry, Settings options, SharedPtr<Reactor> reactor,
         SharedPtr<Logger> logger,
         Callback<Error, SharedPtr<Entry>,
                  SharedPtr<std::map<std::string, std::vector<std::string>>>,
                  Settings, SharedPtr<Reactor>, SharedPtr<Logger>>
               cb) {
    SharedPtr<std::map<std::string, std::vector<std::string>>> fb_service_ips(
          new std::map<std::string, std::vector<std::string>>(
                {{"stun", {}},
                 {"b_api", {}},
                 {"b_graph", {}},
                 {"edge", {}},
                 {"external_cdn", {}},
                 {"scontent_cdn", {}},
                 {"star", {}}}));

    if (error) {
        cb(error, entry, fb_service_ips, options, reactor, logger);
        return;
    }

    size_t names_count = FB_SERVICE_HOSTNAMES.size();
    if (names_count == 0) {
        cb(NoError(), entry, fb_service_ips, options, reactor, logger);
        return;
    }
    SharedPtr<size_t> names_tested(new size_t(0));

    auto dns_cb = [=](std::string service, std::string hostname) {
        return [=](Error err, SharedPtr<dns::Message> message) {
            if (!!err) {
                logger->info("fb_messenger: dns error for %s, %s",
                             service.c_str(), hostname.c_str());
            } else {
                for (auto answer : message->answers) {
                    if (answer.ipv4 != "") {
                        logger->info("got ip %s for service %s",
                            answer.ipv4.c_str(), service.c_str());
                        (*fb_service_ips)[service].push_back(answer.ipv4);
                    }
                }
            }
            *names_tested += 1;
            assert(*names_tested <= names_count);
            if (names_count == *names_tested) {
                cb(NoError(), entry, fb_service_ips, options, reactor, logger);
                return;
            }
        };
    };

    for (auto const &service_and_hostname : FB_SERVICE_HOSTNAMES) {
        std::string service = service_and_hostname.first;
        std::string hostname = service_and_hostname.second;
        // Note: we're passing in an empty nameserver, which rests on the
        // assumption that we're using the `system` DNS resolver.
        constexpr const char *nameserver = "";
        templates::dns_query(entry, "A", "IN", hostname, nameserver,
                             dns_cb(service, hostname), options, reactor,
                             logger);
    }
}

static void
tcp_many(Error error, SharedPtr<Entry> entry,
         SharedPtr<std::map<std::string, std::vector<std::string>>> fb_service_ips,
         Settings options, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
         Callback<SharedPtr<Entry>> cb) {

    if (error) {
        cb(entry);
        return;
    }

    size_t ips_count = 0;
    for (auto const &service_and_ips : *fb_service_ips) {
        // skipping stun for now; this is what ooni-probe does
        if (service_and_ips.first == "stun") {
            continue;
        }
        ips_count += service_and_ips.second.size();
    }
    if (ips_count == 0) {
        cb(entry);
        return;
    }
    SharedPtr<size_t> ips_tested(new size_t(0));

    auto tcp_cb = [=](std::string service, std::string ip, uint16_t port,
            bool this_ip_consistent) {
        return [=](Error err, SharedPtr<net::Transport> txp) {
            assert(!!txp);
            Entry current_entry{
                  {"ip", ip}, {"port", port}, {"status", nullptr}};
            if (!!err) {
                logger->info("tcp failure to %s at %s:%d", service.c_str(),
                             ip.c_str(), port);
                current_entry["status"]["success"] = false;
                current_entry["status"]["failure"] = true;
            } else {
                logger->info("tcp success to %s at %s:%d", service.c_str(),
                             ip.c_str(), port);
                if (this_ip_consistent) {
                    (*entry)["facebook_" + service + "_reachable"] = true;
                }
                current_entry["status"]["success"] = true;
                current_entry["status"]["failure"] = false;
            }
            (*entry)["tcp_connect"].push_back(std::move(current_entry));
            *ips_tested += 1;
            assert(*ips_tested <= ips_count);
            if (ips_count == *ips_tested) {
                // if ANY services were TCP unreachable on all consistent IPs,
                // switch this to true.
                (*entry)["facebook_tcp_blocking"] = false;
                for (auto const &service_and_hostname : FB_SERVICE_HOSTNAMES) {
                    std::string service = service_and_hostname.first;
                    if (service == "stun") { continue; }
                    bool consistent =
                        !!(*entry)["facebook_" + service + "_dns_consistent"];
                    bool reachable =
                        !!(*entry)["facebook_" + service + "_reachable"];
                    logger->info("service %s DNS consistency: %s",
                        service.c_str(), (!!consistent) ? "true" : "false");
                    logger->info("service %s TCP reachability: %s",
                        service.c_str(), (!!reachable) ? "true" : "false");
                    if (consistent && !reachable) {
                        (*entry)["facebook_tcp_blocking"] = true;
                    }
                }
                txp->close([=] {
                    cb(entry);
                });
            } else {
                txp->close([=] {});
            }
        };
    };

    // if we can TCP connect to ANY consistent IP for a service,
    // switch this to true.
    for (auto const &service_and_hostname : FB_SERVICE_HOSTNAMES) {
        std::string service = service_and_hostname.first;
        (*entry)["facebook_" + service + "_reachable"] = false;
    }
    // we're skipping stun like ooni-probe does
    (*entry)["facebook_stun_reachable"] = nullptr;


    for (auto const &service_and_ips : *fb_service_ips) {
        std::string service = service_and_ips.first;
        // if ANY ips for this service are in FB's ASN, switch to true
        (*entry)["facebook_" + service + "_dns_consistent"] = false;
        for (auto const ip : service_and_ips.second) {
            bool this_ip_consistent = ip_in_fb_asn(options, ip);
            if (this_ip_consistent) {
                logger->info("%s is in FB's ASN", ip.c_str());
                (*entry)["facebook_" + service + "_dns_consistent"] = true;
            } else {
                logger->info("%s is NOT in FB's ASN", ip.c_str());
            }
            // not TCPing to stun, because it's UDP
            if (service == "stun") {
                continue;
            }
            Settings tcp_options{options};
            tcp_options["host"] = ip;
            uint16_t port = 443;
            tcp_options["port"] = port;
            tcp_options["net/timeout"] = 10.0;
            templates::tcp_connect(tcp_options,
                tcp_cb(service, ip, port, this_ip_consistent), reactor, logger);
        }
    }
    // if ANY services are not DNS consistent, switch this to true
    (*entry)["facebook_dns_blocking"] = false;
    for (auto const &service_and_hostname : FB_SERVICE_HOSTNAMES) {
        std::string service = service_and_hostname.first;
        bool consistent =
            !!(*entry)["facebook_" + service + "_dns_consistent"];
        logger->info("service %s DNS consistency: %s", service.c_str(),
                (!!consistent) ? "true" : "false");
        if (!consistent) {
            (*entry)["facebook_dns_blocking"] = true;
        }
    }


}

void facebook_messenger(Settings options, Callback<SharedPtr<report::Entry>> callback,
                        SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    logger->info("starting facebook_messenger");
    SharedPtr<Entry> entry(new Entry);
    mk::fcompose(mk::fcompose_policy_async(), dns_many, tcp_many)(
          NoError(), entry, options, reactor, logger, callback);
}

} // namespace ooni
} // namespace mk
