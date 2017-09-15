// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/ooni/constants.hpp"
#include "private/ooni/utils.hpp"
#include <measurement_kit/common/detail/parallel.hpp>
#include <measurement_kit/common/detail/utils.hpp>
#include <measurement_kit/common/detail/waterfall.hpp>
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

class MessengerCtx {
  public:
    SharedPtr<Entry> entry{new Entry};
    std::map<std::string, std::vector<std::string>> fb_service_ips{
        {"stun", {}}, {"b_api", {}},        {"b_graph", {}},
        {"edge", {}}, {"external_cdn", {}}, {"scontent_cdn", {}},
        {"star", {}}};
    SharedPtr<Logger> logger;
    Settings options;
    SharedPtr<Reactor> reactor;
};

static Continuation<Error> dns_many(SharedPtr<MessengerCtx> ctx) {
    return [=](Callback<Error> &&cb) {
        // if we find any inconsistent DNS later, switch this to true
        (*ctx->entry)["facebook_dns_blocking"] = false;

        size_t names_count = FB_SERVICE_HOSTNAMES.size();
        if (names_count == 0) {
            cb(NoError());
            return;
        }
        ParallelCallback parallel_callback{names_count, std::move(cb)};

        auto dns_cb = [=](std::string service, std::string hostname) {
            return [=](Error err, SharedPtr<dns::Message> message) {
                if (!!err) {
                    ctx->logger->info("fb_messenger: dns error for %s, %s",
                                      service.c_str(), hostname.c_str());
                } else {
                    for (auto answer : message->answers) {
                        if ((answer.ipv4 != "") || (answer.hostname != "")) {
                            std::string asn_p = ctx->options.get(
                                "geoip_asn_path", std::string{});
                            auto geoip =
                                GeoipCache::thread_local_instance()->get(asn_p);
                            ErrorOr<std::string> asn =
                                geoip->resolve_asn(answer.ipv4);
                            if (!!asn && asn.as_value() != "AS0") {
                                ctx->logger->info("%s ipv4: %s, %s",
                                                  hostname.c_str(),
                                                  answer.ipv4.c_str(),
                                                  asn.as_value().c_str());
                                // if consistent, add to list for tcp connect
                                // later
                                if (asn.as_value() == "AS32934") {
                                    ctx->fb_service_ips[service].push_back(
                                        answer.ipv4);
                                } else {
                                    (*ctx->entry)["facebook_dns_blocking"] =
                                        true;
                                }
                            }
                        }
                    }
                    // if any consistent IPs, consistent = true
                    (*ctx->entry)["facebook_" + service + "_dns_consistent"] =
                        !ctx->fb_service_ips[service].empty();
                }
                // XXX: I believe here we should actually return error?
                parallel_callback(NoError());
            };
        };

        for (auto const &service_and_hostname : FB_SERVICE_HOSTNAMES) {
            std::string service = service_and_hostname.first;
            std::string hostname = service_and_hostname.second;
            // Note: we're passing in an empty nameserver, which rests on the
            // assumption that we're using the `system` DNS resolver.
            constexpr const char *nameserver = "";
            templates::dns_query(ctx->entry, "A", "IN", hostname, nameserver,
                                 dns_cb(service, hostname), ctx->options,
                                 ctx->reactor, ctx->logger);
        }
    };
}

static Continuation<Error> tcp_many(SharedPtr<MessengerCtx> ctx) {
    return [=](Callback<Error> &&cb) {

        // if we find any blocked TCP later, switch this to true
        (*ctx->entry)["facebook_tcp_blocking"] = false;

        // Note: we're skipping stun like ooni-probe does
        (*ctx->entry)["facebook_stun_reachable"] = nullptr;

        size_t ips_count = 0;
        for (auto const &service_and_ips : ctx->fb_service_ips) {
            // skipping stun for now; this is what ooni-probe does
            if (service_and_ips.first == "stun") {
                continue;
            }
            ips_count += service_and_ips.second.size();
        }
        if (ips_count == 0) {
            cb(NoError());
            return;
        }
        ParallelCallback parallel_callback{ips_count, std::move(cb)};

        auto tcp_cb = [=](std::string service, std::string ip, uint16_t port) {
            return [=](Error err, SharedPtr<net::Transport> txp) {
                assert(!!txp);
                Entry current_entry{
                    {"ip", ip}, {"port", port}, {"status", nullptr}};
                if (!!err) {
                    ctx->logger->info("tcp failure to %s at %s:%d",
                                      service.c_str(), ip.c_str(), port);
                    (*ctx->entry)["facebook_" + service + "_reachable"] = false;
                    (*ctx->entry)["facebook_tcp_blocking"] = true;
                    current_entry["status"]["success"] = false;
                    current_entry["status"]["failure"] = true;
                } else {
                    ctx->logger->info("tcp success to %s at %s:%d",
                                      service.c_str(), ip.c_str(), port);
                    (*ctx->entry)["facebook_" + service + "_reachable"] = true;
                    current_entry["status"]["success"] = true;
                    current_entry["status"]["failure"] = false;
                }
                (*ctx->entry)["tcp_connect"].push_back(
                    std::move(current_entry));
                txp->close(nullptr);
                parallel_callback(NoError());
            };
        };

        for (auto const &service_and_ips : ctx->fb_service_ips) {
            std::string service = service_and_ips.first;
            // skipping stun for now; this is what ooni-probe does
            if (service == "stun") {
                continue;
            }
            for (auto const &ip : service_and_ips.second) {
                Settings tcp_options{ctx->options};
                tcp_options["host"] = ip;
                uint16_t port = 443;
                tcp_options["port"] = port;
                tcp_options["net/timeout"] = 10.0;
                templates::tcp_connect(tcp_options, tcp_cb(service, ip, port),
                                       ctx->reactor, ctx->logger);
            }
        }
    };
}

void facebook_messenger(Settings options, Callback<SharedPtr<report::Entry>> cb,
                        SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    logger->info("starting facebook_messenger");
    SharedPtr<MessengerCtx> ctx{new MessengerCtx};
    ctx->options = options;
    ctx->reactor = reactor;
    ctx->logger = logger;
    WaterfallExecutor{[=](Error) { cb(ctx->entry); }}
        .add(dns_many(ctx))
        .add(tcp_many(ctx))
        .start();
}

} // namespace ooni
} // namespace mk
