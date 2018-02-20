// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ooni/constants.hpp"
#include "src/libmeasurement_kit/ooni/whatsapp.hpp"
#include "src/libmeasurement_kit/common/fcompose.hpp"
#include "src/libmeasurement_kit/common/parallel.hpp"
#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/ooni/templates.hpp"
#include <cassert>
#include <measurement_kit/net/utils.hpp>
#include <measurement_kit/report.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

typedef std::map<std::string, std::vector<std::string>> host_to_ips_t;

// Note: we used to gather information on WhatsApp networks using
// <https://www.whatsapp.com/cidr.txt> but now that URL does not
// provide WhatsApp IP information anymore. Thus, the check whether
// an IP lies in WhatsApp range is partly outdated.
std::vector<std::string> WHATSAPP_NETS = {{"31.13.64.51/32"},
        {"31.13.65.49/32"}, {"31.13.66.49/32"}, {"31.13.67.51/32"},
        {"31.13.68.52/32"}, {"31.13.69.240/32"}, {"31.13.70.49/32"},
        {"31.13.71.49/32"}, {"31.13.72.52/32"}, {"31.13.73.49/32"},
        {"31.13.74.49/32"}, {"31.13.75.52/32"}, {"31.13.76.81/32"},
        {"31.13.77.49/32"}, {"31.13.78.53/32"}, {"31.13.80.53/32"},
        {"31.13.81.53/32"}, {"31.13.82.51/32"}, {"31.13.83.51/32"},
        {"31.13.84.51/32"}, {"31.13.85.51/32"}, {"31.13.86.51/32"},
        {"31.13.87.51/32"}, {"31.13.88.49/32"}, {"31.13.90.51/32"},
        {"31.13.91.51/32"}, {"31.13.92.52/32"}, {"31.13.93.51/32"},
        {"31.13.94.52/32"}, {"31.13.95.63/32"}, {"50.22.198.204/30"},
        {"50.22.210.32/30"}, {"50.22.210.128/27"}, {"50.22.225.64/27"},
        {"50.22.235.248/30"}, {"50.22.240.160/27"}, {"50.23.90.128/27"},
        {"50.97.57.128/27"}, {"75.126.39.32/27"}, {"108.168.171.224/27"},
        {"108.168.174.0/27"}, {"108.168.176.192/26"}, {"108.168.177.0/27"},
        {"108.168.180.96/27"}, {"108.168.254.65/32"}, {"108.168.255.224/32"},
        {"108.168.255.227/32"}, {"157.240.0.53/32"}, {"157.240.2.53/32"},
        {"157.240.3.53/32"}, {"157.240.7.54/32"}, {"158.85.0.96/27"},
        {"158.85.5.192/27"}, {"158.85.46.128/27"}, {"158.85.48.224/27"},
        {"158.85.58.0/25"}, {"158.85.61.192/27"}, {"158.85.224.160/27"},
        {"158.85.233.32/27"}, {"158.85.249.128/27"}, {"158.85.254.64/27"},
        {"169.44.36.0/25"}, {"169.44.57.64/27"}, {"169.44.58.64/27"},
        {"169.44.80.0/26"}, {"169.44.82.96/27"}, {"169.44.82.128/27"},
        {"169.44.82.192/26"}, {"169.44.83.0/26"}, {"169.44.83.96/27"},
        {"169.44.83.128/27"}, {"169.44.83.192/26"}, {"169.44.84.0/24"},
        {"169.44.85.64/27"}, {"169.45.71.32/27"}, {"169.45.71.96/27"},
        {"169.45.87.128/26"}, {"169.45.169.192/27"}, {"169.45.182.96/27"},
        {"169.45.210.64/27"}, {"169.45.214.224/27"}, {"169.45.219.224/27"},
        {"169.45.237.192/27"}, {"169.45.238.32/27"}, {"169.45.248.96/27"},
        {"169.45.248.160/27"}, {"169.46.52.224/27"}, {"169.47.5.192/26"},
        {"169.53.29.128/27"}, {"169.53.48.32/27"}, {"169.53.71.224/27"},
        {"169.53.81.64/27"}, {"169.53.250.128/26"}, {"169.53.252.64/27"},
        {"169.53.255.64/27"}, {"169.54.2.160/27"}, {"169.54.44.224/27"},
        {"169.54.51.32/27"}, {"169.54.55.192/27"}, {"169.54.193.160/27"},
        {"169.54.210.0/27"}, {"169.54.222.128/27"}, {"169.55.67.224/27"},
        {"169.55.69.128/26"}, {"169.55.74.32/27"}, {"169.55.75.96/27"},
        {"169.55.126.64/26"}, {"169.55.210.96/27"}, {"169.55.235.160/27"},
        {"173.192.162.32/27"}, {"173.192.219.128/27"}, {"173.192.222.160/27"},
        {"173.192.231.32/27"}, {"173.192.234.96/27"}, {"173.193.198.96/27"},
        {"173.193.205.0/27"}, {"173.193.230.96/27"}, {"173.193.230.128/27"},
        {"173.193.230.192/27"}, {"173.193.239.0/27"}, {"174.36.208.128/27"},
        {"174.36.210.32/27"}, {"174.36.251.192/27"}, {"174.37.199.192/27"},
        {"174.37.217.64/27"}, {"174.37.243.64/27"}, {"174.37.251.0/27"},
        {"179.60.192.51/32"}, {"179.60.193.51/32"}, {"179.60.195.51/32"},
        {"184.173.136.64/27"}, {"184.173.147.32/27"}, {"184.173.161.64/32"},
        {"184.173.161.160/27"}, {"184.173.173.116/32"}, {"184.173.179.32/27"},
        {"185.60.216.53/32"}, {"185.60.218.53/32"}, {"192.155.212.192/27"},
        {"198.11.193.182/31"}, {"198.11.251.32/27"}, {"198.23.80.0/27"},
        {"208.43.115.192/27"}, {"208.43.117.79/32"}, {"208.43.122.128/27"},
        {"2607:f0d0:1b01:d4::/64"}, {"2607:f0d0:1b02:14d::/64"},
        {"2607:f0d0:1b04:32::/64"}, {"2607:f0d0:1b04:bb::/64"},
        {"2607:f0d0:1b04:bc::/64"}, {"2607:f0d0:1b06::/64"},
        {"2607:f0d0:1b06:4::/64"}, {"2607:f0d0:1e01:b1::/64"},
        {"2607:f0d0:2102:229::/64"}, {"2607:f0d0:2601:37::/64"},
        {"2607:f0d0:3003:1bc::/64"}, {"2607:f0d0:3003:1cd::/64"},
        {"2607:f0d0:3004:136::/64"}, {"2607:f0d0:3004:174::/64"},
        {"2607:f0d0:3005:183::/64"}, {"2607:f0d0:3005:1a3::/64"},
        {"2607:f0d0:3006:84::/64"}, {"2607:f0d0:3006:af::/64"},
        {"2607:f0d0:3801:38::/64"}, {"2607:f0d0:3801:14b::/64"},
        {"2607:f0d0:3802:48::/64"}, {"2a03:2880:f200:c5:face:b00c::167/128"},
        {"2a03:2880:f200:1c5:face:b00c::167/128"},
        {"2a03:2880:f201:c5:face:b00c::167/128"},
        {"2a03:2880:f202:c4:face:b00c::167/128"},
        {"2a03:2880:f203:c5:face:b00c::167/128"},
        {"2a03:2880:f204:c5:face:b00c::167/128"},
        {"2a03:2880:f205:c5:face:b00c::167/128"},
        {"2a03:2880:f206:c5:face:b00c::167/128"},
        {"2a03:2880:f207:c5:face:b00c::167/128"},
        {"2a03:2880:f208:c5:face:b00c::167/128"},
        {"2a03:2880:f209:c5:face:b00c::167/128"},
        {"2a03:2880:f20a:c5:face:b00c::167/128"},
        {"2a03:2880:f20b:c5:face:b00c::167/128"},
        {"2a03:2880:f20c:c6:face:b00c::167/128"},
        {"2a03:2880:f20d:c5:face:b00c::167/128"},
        {"2a03:2880:f20e:c5:face:b00c::167/128"},
        {"2a03:2880:f20f:c6:face:b00c::167/128"},
        {"2a03:2880:f210:c5:face:b00c::167/128"},
        {"2a03:2880:f211:c5:face:b00c::167/128"},
        {"2a03:2880:f212:c5:face:b00c::167/128"},
        {"2a03:2880:f213:c5:face:b00c::167/128"},
        {"2a03:2880:f213:80c5:face:b00c::167/128"},
        {"2a03:2880:f214:c5:face:b00c::167/128"},
        {"2a03:2880:f215:c5:face:b00c::167/128"},
        {"2a03:2880:f216:c5:face:b00c::167/128"},
        {"2a03:2880:f217:c5:face:b00c::167/128"},
        {"2a03:2880:f218:c3:face:b00c::167/128"},
        {"2a03:2880:f219:c5:face:b00c::167/128"},
        {"2a03:2880:f21a:c5:face:b00c::167/128"},
        {"2a03:2880:f21b:c5:face:b00c::167/128"},
        {"2a03:2880:f21c:c5:face:b00c::167/128"},
        {"2a03:2880:f21c:80c5:face:b00c::167/128"},
        {"2a03:2880:f21f:c5:face:b00c::167/128"},
        {"2a03:2880:f221:c5:face:b00c::167/128"},
        {"2a03:2880:f222:c5:face:b00c::167/128"},
        {"2a03:2880:f223:c5:face:b00c::167/128"},
        {"2a03:2880:f225:c4:face:b00c::167/128"},
        {"2a03:2880:f226:c6:face:b00c::167/128"},
        {"2a03:2880:f227:c5:face:b00c::167/128"}};

std::vector<uint8_t> ip_to_bytes(std::string ip) {
    sockaddr_storage ip_storage;
    socklen_t ip_sz;
    memset(&ip_storage, 0, sizeof ip_storage);

    constexpr uint16_t port = 0;
    Error err = mk::net::make_sockaddr(ip, port, &ip_storage, &ip_sz);
    if (!!err) {
        return {};
    }

    std::vector<uint8_t> bs;
    uint32_t bsv4;
    uint8_t *bsv6;
    if (ip_sz == 16) { /* IPv4 */
        bsv4 = ((struct sockaddr_in *)&ip_storage)->sin_addr.s_addr;
        for (int i = 0; i < 4; i++) {
            bs.insert(bs.end(), (bsv4 & 0xff));
            bsv4 >>= 8;
        }
    } else if (ip_sz == 28) { /* IPv6 */
        bsv6 = ((struct sockaddr_in6 *)&ip_storage)->sin6_addr.s6_addr;
        for (int i = 0; i < 16; i++) {
            bs.insert(bs.end(), bsv6[i]);
        }
    } else {
        throw std::runtime_error("unexpected ip_sz");
    }

    return bs;
}

ErrorOr<bool> same_pre(
        std::vector<uint8_t> ip1, std::vector<uint8_t> ip2, int pre_bits) {
    if (ip1.size() != ip2.size()) {
        return {GenericError(), {}};
    }
    // TODO: check pre_bits is sane
    // full prefix bytes
    int i = 0;
    for (; i < (pre_bits / 8); i++) {
        assert(i >= 0 && (size_t)i < ip1.size() && (size_t)i < ip2.size());
        if (ip1[i] != ip2[i]) {
            return {NoError(), false};
        }
    }

    int rem_bits = pre_bits % 8;
    if (rem_bits == 0) {
        return {NoError(), true};
    }

    assert(i >= 0 && (size_t)i < ip1.size() && (size_t)i < ip2.size());
    if ((ip1[i] >> (8 - rem_bits)) != (ip2[i] >> (8 - rem_bits))) {
        return {NoError(), false};
    }
    return {NoError(), true};
}

ErrorOr<bool> ip_in_net(std::string ip1, std::string ip_w_mask) {
    auto ip1bs = ip_to_bytes(ip1);
    auto ip2 = mk::split<std::vector<std::string>>(ip_w_mask, "/");
    assert(ip2.size() == 2);
    auto ip2bs = ip_to_bytes(ip2[0]);
    auto pre_bits = std::stoi(ip2[1]);
    return same_pre(ip1bs, ip2bs, pre_bits);
}

bool ip_in_nets(std::string ip, std::vector<std::string> nets) {
    for (auto net : nets) {
        auto res = ip_in_net(ip, net);
        if (!!res && (res.as_value() == true)) {
            return true;
        }
    }
    return false;
}

static void tcp_many(host_to_ips_t host_to_ips, SharedPtr<Entry> entry,
        Settings options, SharedPtr<Reactor> reactor,
        SharedPtr<Logger> logger, Callback<Error> cb) {
    // a hostname is DNS inconsistent if ALL of its IPs are inconsistent.
    (*entry)["whatsapp_endpoints_dns_inconsistent"] = Entry::array();
    // on first TCP success of a consistent IP, set to "ok".
    (*entry)["whatsapp_endpoints_status"] = "blocked";
    // all hostnames start here;removed upon a TCP success to a consistent IP
    SharedPtr<std::set<std::string>> blocked_hostnames(
          new std::set<std::string>);
    // at the end, we copy these ^ hostnames into the report:
    (*entry)["whatsapp_endpoints_blocked"] = Entry::array();
    size_t ips_count = 0;
    SharedPtr<size_t> ips_tested(new size_t(0));
    for (auto const& hostname_ipv : host_to_ips) {
        blocked_hostnames->insert(hostname_ipv.first);
        ips_count += hostname_ipv.second.size() * 2; // two ports per IP
    }

    auto tcp_cb = [=](std::string hostname, std::string ip, int port,
                      bool this_ip_consistent) {
        return [=](Error connect_err, SharedPtr<net::Transport> txp) {
            Entry result = {
                    {"ip", ip}, {"port", port},
                    {"status", {{"success", nullptr}, {"failure", nullptr}}},
            };
            if (!!connect_err) {
                logger->info("tcp failure to %s:%d", ip.c_str(), port);
                result["status"]["success"] = false;
                result["status"]["failure"] = connect_err.reason;
            } else {
                logger->info("tcp success to %s:%d", ip.c_str(), port);
                result["status"]["success"] = true;
                result["status"]["failure"] = false;
                if (this_ip_consistent) {
                    logger->info("removing %s from blocked_hostnames",
                            hostname.c_str());
                    (*entry)["whatsapp_endpoints_status"] = "ok";
                    blocked_hostnames->erase(hostname);
                }
            }
            (*entry)["tcp_connect"].push_back(result);
            txp->close(nullptr);
            *ips_tested += 1;
            if (ips_count == *ips_tested) {
                for (auto const& hostname : *blocked_hostnames) {
                    (*entry)["whatsapp_endpoints_blocked"].push_back(hostname);
                }
                cb(NoError());
            }
        };
    };

    // this is needed in case dns_many() returns no ips
    if (ips_count == 0) {
        cb(NoError());
        return;
    }
    for (auto const& hostname_ipv : host_to_ips) {
        std::string hostname = hostname_ipv.first;
        // if *any* ips are in our old view of the whatsapp network,
        // switch this to true.
        bool this_host_consistent = false;
        for (auto const& ip : hostname_ipv.second) {
            bool this_ip_consistent;
            if (ip_in_nets(ip, WHATSAPP_NETS)) {
                logger->info("%s seems to belong to Whatsapp", ip.c_str());
                this_host_consistent = true;
                this_ip_consistent = true;
            } else {
                logger->info("%s seems to NOT belong to Whatsapp", ip.c_str());
                this_ip_consistent = false;
            }
            // XXX hardcoded
            std::vector<int> ports{443, 5222};
            for (auto const &port : ports) {
                Settings local_options = options;
                local_options["host"] = ip;
                local_options["port"] = port;
                templates::tcp_connect(
                    local_options,
                    tcp_cb(hostname, ip, port, this_ip_consistent),
                    reactor, logger);
            }
        }
        if (!this_host_consistent) {
            (*entry)["whatsapp_endpoints_dns_inconsistent"].push_back(hostname);
            // "whatsapp_endpoints_blocked" means there were one or more
            // consistent IPs, but we couldn't connect to any of the
            // consistent IPs. (if there are no consistent IPs, we don't
            // put it in the "[tcp_]blocked" list.)
            blocked_hostnames->erase(hostname);
        }
    }
}

static void dns_many(std::vector<std::string> hostnames, SharedPtr<Entry> entry,
        Settings options, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
        Callback<Error, host_to_ips_t> cb) {
    int names_count = hostnames.size();
    logger->info("whatsapp: %d hostnames", names_count);
    SharedPtr<host_to_ips_t> host_to_ips(new host_to_ips_t);
    SharedPtr<int> names_tested(new int(0));

    auto dns_cb = [=](std::string hostname) {
        return [=](Error err, SharedPtr<dns::Message> message) {
            if (!!err) {
                logger->info("whatsapp: dns error for %s", hostname.c_str());
            } else {
                std::vector<std::string> this_ips; // will check if they are consistent
                for (auto answer : message->answers) {
                    if (answer.ipv4 != "") {
                        logger->info("(1) %s ipv4: %s", hostname.c_str(),
                                answer.ipv4.c_str());
						this_ips.push_back(answer.ipv4);
                    } else if (answer.hostname != "") {
                        logger->info("(2) %s hostname: %s", hostname.c_str(),
                                answer.hostname.c_str());
                    } else {
                        /* NOTHING */;
                    }
                }
                for (auto ip : this_ips) {
                    (*host_to_ips)[hostname].push_back(ip);
                }
            }
            *names_tested += 1;
            if (names_count == *names_tested) {
                size_t ips_count = 0;
                for (auto const& hostname_ipv : *host_to_ips) {
                    ips_count += hostname_ipv.second.size();
                }
                logger->info("dns_many() found %d ips", (int)ips_count);
                cb(NoError(), *host_to_ips);
            }
        };
    };

    for (auto const &hostname : hostnames) {
        // This will work as long as we are using the system resolver for which
        // it does not make any sense to specify a DNS nameserver.
        constexpr const char *nameserver = "";
        templates::dns_query(entry, "A", "IN", hostname, nameserver,
                dns_cb(hostname), options, reactor, logger);
    }
}

static void http_many(const std::vector<std::string> urls,
        std::string resource_name, SharedPtr<Entry> entry, Settings options,
        SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
        Callback<Error> cb) {
    // resource_name is "registration_server" or "whatsapp_web"
    // if ANYthing is blocked, we consider the resource blocked
    (*entry)[resource_name + "_status"] = "ok";
    (*entry)[resource_name + "_failure"] = nullptr;
    int urls_count = urls.size();
    SharedPtr<int> urls_tested(new int(0));

    auto http_cb = [=](std::string url) {
        return [=](Error err, SharedPtr<http::Response> /*response*/) {
            if (!!err) {
                logger->info(
                        "whatsapp: failure HTTP connecting to %s", url.c_str());
                (*entry)[resource_name + "_status"] = "blocked";
                (*entry)[resource_name + "_failure"] = err.reason;
            } else {
                logger->info(
                        "whatsapp: success HTTP connecting to %s", url.c_str());
            }
            *urls_tested += 1;
            if (urls_count == *urls_tested) {
                cb(NoError());
            }
        };
    };

    for (auto url : urls) {
        // Note: options are copied, so each run of the template will
        // in the end get its own URL
        options["http/url"] = url;
        http::Headers headers = constants::COMMON_CLIENT_HEADERS;
        std::string body; // Empty body in this case
        templates::http_request(
                entry, options, headers, body, http_cb(url), reactor, logger);
    }
}

void whatsapp(Settings options, Callback<SharedPtr<report::Entry>> callback,
        SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    std::vector<std::string> WHATSAPP_REG_URLS = {
            "https://v.whatsapp.net/v2/register"};
    std::vector<std::string> WHATSAPP_WEB_URLS = {
            "https://web.whatsapp.com/", "http://web.whatsapp.com/"};
    // XXX generate these with a range or something
    std::vector<std::string> WHATSAPP_ENDPOINT_HOSTNAMES = {"e1.whatsapp.net",
            "e2.whatsapp.net", "e3.whatsapp.net", "e4.whatsapp.net",
            "e5.whatsapp.net", "e6.whatsapp.net", "e7.whatsapp.net",
            "e8.whatsapp.net", "e9.whatsapp.net", "e10.whatsapp.net",
            "e11.whatsapp.net", "e12.whatsapp.net", "e13.whatsapp.net",
            "e14.whatsapp.net", "e15.whatsapp.net", "e16.whatsapp.net"};
    std::vector<std::string> whatsapp_endpoint_hostnames;
    if (!!options.get("all_endpoints", false)) {
        logger->info("doing all endpoints");
        whatsapp_endpoint_hostnames = WHATSAPP_ENDPOINT_HOSTNAMES;
    } else {
        logger->info("doing a random endpoint");
        whatsapp_endpoint_hostnames.push_back(
                random_choice(WHATSAPP_ENDPOINT_HOSTNAMES));
    }

    logger->info("starting whatsapp test");
    SharedPtr<Entry> entry(new Entry);

    mk::fcompose(mk::fcompose_policy_async(),
            [=](Callback<> cb) {
                http_many(WHATSAPP_REG_URLS, "registration_server", entry,
                        options, reactor, logger, [=](Error err) {
                            logger->info(
                                    "saw %s in Whatsapp's registration server",
                                    (!!err) ? "at least one error"
                                            : "no errors");
                            cb();
                        });
            },
            [=](Callback<> cb) {
                http_many(WHATSAPP_WEB_URLS, "whatsapp_web", entry, options,
                        reactor, logger, [=](Error err) {
                            logger->info("saw %s in Whatsapp Web",
                                    (!!err) ? "at least one error"
                                            : "no errors");
                            cb();
                        });
            },
            [=](Callback<host_to_ips_t> cb) {
                dns_many(whatsapp_endpoint_hostnames, entry, options, reactor,
                        logger, [=](Error err, host_to_ips_t host_to_ips) {
                            logger->info("saw %s in Whatsapp's endpoints (DNS)",
                                    (!!err) ? "at least one error"
                                            : "no errors");
                            cb(host_to_ips);
                        });
            },
            [=](host_to_ips_t host_to_ips,
                Callback<> cb) {
                tcp_many(host_to_ips, entry, options, reactor, logger, [=](Error err) {
                    logger->info("saw %s in Whatsapp's endpoints (TCP)",
                            (!!err) ? "at least one error" : "no errors");
                    cb();
                });
            })([=]() {
        logger->debug("calling final callback");
        callback(entry);
    });
}

} // namespace ooni
} // namespace mk
