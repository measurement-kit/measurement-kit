// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"
#include "../ooni/constants.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

static void tcp_many(const std::vector<std::string> ip_ports,
                     Var<Entry> entry,
                     Callback<Error> cb,
                     Var<Reactor> reactor,
                     Var<Logger> logger) {
    // C++20 should have continuations in std; until then,
    // we copy the ghetto pattern from web_connectivity
    int ips_count = ip_ports.size();
    Var<int> ips_tested(new int(0));
    if (ips_count == *ips_tested) {
        cb(NoError());
        return;
    }

    // per-endpoint, we only consider success/failure.
    // telegram is "blocked" if no endpoints succeed.
    auto tcp_cb = [=](std::string ip, int port) {
        return [=](Error err, Var<net::Transport> txp) {
            bool close_txp = true; // if connected, we must disconnect
            Entry result = {
                {"ip", ip},
                {"port", port},
                {"status",
                    {{"success", nullptr},
                    {"failure", nullptr}}},
            };
            if (!!err) {
                logger->info("telegram: failure TCP connecting to %s:%d",
                             ip.c_str(), port);
                result["status"]["success"] = false;
                result["status"]["failure"] = err.as_ooni_error();
                close_txp = false;
            } else {
                logger->info("telegram: success TCP connecting to %s:%d",
                             ip.c_str(), port);
                result["status"]["success"] = true;
            }
            (*entry)["tcp_connect"].push_back(result);
            *ips_tested += 1;
            if (ips_count == *ips_tested) {
                if (close_txp == true) {
                    txp->close([=] { cb(NoError()); });
                } else {
                    cb(NoError());
                }
            } else {
                if (close_txp == true) {
                    // XXX optimistic closure
                    txp->close([=] {});
                }
            }

        };
    };

    for (auto ip_port : ip_ports) {
        std::list<std::string> ip_port_l = split(ip_port, ":");
        if (ip_port_l.size() != 2) {
            logger->warn("Couldn't split ip_port: %s", ip_port.c_str());
            // (*entry)["failure"] = ValueError().as_ooni_error();
            // callback(entry);
            return;
        }
        std::string ip = ip_port_l.front();
        int port = std::stoi(ip_port_l.back());

        Settings tcp_options;
        tcp_options["host"] = ip;
        tcp_options["port"] = port;
        tcp_options["net/timeout"] = 10.0;
        templates::tcp_connect(tcp_options, tcp_cb(ip, port), reactor, logger);
    }

}

static void http_many(const std::vector<std::string> urls,
                      Var<Entry> entry,
                      Callback<Error> cb,
                      Var<Reactor> reactor,
                      Var<Logger> logger) {
    int urls_count = urls.size();
    Var<int> urls_tested(new int(0));
    if (urls_count == *urls_tested) {
        cb(NoError());
        return;
    }

    auto http_cb = [=](std::string url) {
        return [=](Error err, Var<http::Response> response) {
             Entry result = {
                 {"url", url},
                 {"status",
                     {{"success", nullptr},
                     {"failure", nullptr}}},
             };
             if (!!err) {
                 logger->info("telegram: failure HTTP connecting to %s",
                     url.c_str());
                 result["status"]["success"] = false;
                 result["status"]["failure"] = err.as_ooni_error();
             } else {
                 logger->info("telegram: success HTTP connecting to %s",
                     url.c_str());
                 result["status"]["success"] = true;
             }
            (*entry)["http_connect"].push_back(result);
             *urls_tested += 1;
             if (urls_count == *urls_tested) {
                 cb(NoError());
             }
         };
    };

    for (auto url : urls) {
        Settings http_options;
        http_options["http/url"] = url;
        http::Headers headers = constants::COMMON_CLIENT_HEADERS;
        std::string body;
        templates::http_request(entry, http_options, headers, body,
                                http_cb(url), reactor, logger);
    }

}

void telegram(std::string input, Settings options,
              Callback<Var<report::Entry>> callback,
              Var<Reactor> reactor, Var<Logger> logger) {
    std::vector<std::string>
        TELEGRAM_WEB_URLS = { "http://web.telegram.org/",
                              "https://web.telegram.org/" };
    // should probably just make these std::pair<std::string,int>,
    // but I'm not sure if this will be better later when I get
    // rid of the duplication between this and the http ones
    std::vector<std::string>
        TELEGRAM_TCP_ENDPOINTS = { "149.154.175.50:80",
                                   "149.154.175.50:443",
                                   "149.154.167.51:80",
                                   "149.154.167.51:443",
                                   "149.154.175.100:80",
                                   "149.154.175.100:443",
                                   "149.154.167.91:80",
                                   "149.154.167.91:443",
                                   "149.154.171.5:80",
                                   "149.154.171.5:443" };

    // duplication for now to make things easier...
    std::vector<std::string>
        TELEGRAM_HTTP_ENDPOINTS = { "http://149.154.175.50:80",
                                    "http://149.154.175.50:443",
                                    "http://149.154.167.51:80",
                                    "http://149.154.167.51:443",
                                    "http://149.154.175.100:80",
                                    "http://149.154.175.100:443",
                                    "http://149.154.167.91:80",
                                    "http://149.154.167.91:443",
                                    "http://149.154.171.5:80",
                                    "http://149.154.171.5:443" };

    logger->info("starting telegram test");
    Var<Entry> entry(new Entry);

    // first try telegram web
    http_many(TELEGRAM_WEB_URLS, entry, [=](Error err) {
        logger->info("done testing telegram web");
        // then try endpoints as TCP
        tcp_many(TELEGRAM_TCP_ENDPOINTS, entry, [=](Error err){
            logger->info("done testing endpoints as TCP");
            // then try endpoints as HTTP
            http_many(TELEGRAM_HTTP_ENDPOINTS, entry, [=](Error err){
                logger->info("done testing endpoints as HTTP");
                // then finish!
                callback(entry);
            }, reactor, logger);
        }, reactor, logger);
    }, reactor, logger);

    return;
}

} // namespace ooni
} // namespace mk
