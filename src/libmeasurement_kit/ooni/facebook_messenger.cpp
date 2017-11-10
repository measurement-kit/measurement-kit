// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/ooni/constants.hpp"
#include "private/ooni/utils.hpp"
#include "private/common/fcompose.hpp"
#include "private/common/utils.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

static const std::map<std::string, std::string> &FB_SERVICE_HOSTNAMES = {
      {"stun", "stun.fbsbx.com"},
      {"b_api", "b-api.facebook.com"},
      {"b_graph", "b-graph.facebook.com"},
      {"edge", "edge-mqtt.facebook.com"},
      {"external_cdn", "external.xx.fbcdn.net"},
      {"scontent_cdn", "scontent.xx.fbcdn.net"},
      {"star", "star.c10r.facebook.com"}};

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

    // if we find any inconsistent DNS later, switch this to true
    (*entry)["facebook_dns_blocking"] = false;

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
                    if ((answer.ipv4 != "") || (answer.hostname != "")) {
                        std::string asn_p =
                              options.get("geoip_asn_path", std::string{});
                        auto geoip =
                              GeoipCache::thread_local_instance()->get(asn_p);
                        ErrorOr<std::string> asn =
                              geoip->resolve_asn(answer.ipv4);
                        if (!!asn && asn.as_value() != "AS0") {
                            logger->info("%s ipv4: %s, %s", hostname.c_str(),
                                         answer.ipv4.c_str(),
                                         asn.as_value().c_str());
                            // if consistent, add to list for tcp connect later
                            if (asn.as_value() == "AS32934") {
                                (*fb_service_ips)[service].push_back(
                                      answer.ipv4);
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

    // if we find any blocked TCP later, switch this to true
    (*entry)["facebook_tcp_blocking"] = false;

    // Note: we're skipping stun like ooni-probe does
    (*entry)["facebook_stun_reachable"] = nullptr;

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

    auto tcp_cb = [=](std::string service, std::string ip, uint16_t port) {
        return [=](Error err, SharedPtr<net::Transport> txp) {
            assert(!!txp);
            Entry current_entry{
                  {"ip", ip}, {"port", port}, {"status", nullptr}};
            if (!!err) {
                logger->info("tcp failure to %s at %s:%d", service.c_str(),
                             ip.c_str(), port);
                (*entry)["facebook_" + service + "_reachable"] = false;
                (*entry)["facebook_tcp_blocking"] = true;
                current_entry["status"]["success"] = false;
                current_entry["status"]["failure"] = true;
            } else {
                logger->info("tcp success to %s at %s:%d", service.c_str(),
                             ip.c_str(), port);
                (*entry)["facebook_" + service + "_reachable"] = true;
                current_entry["status"]["success"] = true;
                current_entry["status"]["failure"] = false;
            }
            (*entry)["tcp_connect"].push_back(std::move(current_entry));
            *ips_tested += 1;
            assert(*ips_tested <= ips_count);
            if (ips_count == *ips_tested) {
                txp->close([=] { cb(entry); });
            } else {
                txp->close([=] {});
            }
        };
    };

    for (auto const &service_and_ips : *fb_service_ips) {
        std::string service = service_and_ips.first;
        // skipping stun for now; this is what ooni-probe does
        if (service == "stun") {
            continue;
        }
        for (auto const &ip : service_and_ips.second) {
            Settings tcp_options{options};
            tcp_options["host"] = ip;
            uint16_t port = 443;
            tcp_options["port"] = port;
            tcp_options["net/timeout"] = 10.0;
            templates::tcp_connect(tcp_options, tcp_cb(service, ip, port),
                                   reactor, logger);
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
