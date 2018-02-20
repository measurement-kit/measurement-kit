// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/common/fcompose.hpp"
#include "src/libmeasurement_kit/common/parallel.hpp"
#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/ooni/constants.hpp"
#include "src/libmeasurement_kit/ooni/utils.hpp"
#include "src/libmeasurement_kit/ooni/templates.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

static void tcp_many(const std::vector<std::string> ip_ports, SharedPtr<Entry> entry,
        Settings options, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
        Callback<Error> all_done_cb) {
    auto connected_cb = [=](std::string ip, int port, Callback<Error> done_cb) {
        return [=](Error connect_error, SharedPtr<net::Transport> txp) {
            Entry result = {
                {"ip", ip}, {"port", port},
                {"status", {{"success", nullptr}, {"failure", nullptr}}},
            };
            if (!!connect_error) {
                logger->info("telegram: failure TCP connecting to %s:%d",
                    ip.c_str(), port);
                result["status"]["success"] = false;
                result["status"]["failure"] = connect_error.reason;
            } else {
                logger->info("telegram: success TCP connecting to %s:%d",
                    ip.c_str(), port);
                result["status"]["success"] = true;
                (*entry)["telegram_tcp_blocking"] = false;
            }
            (*entry)["tcp_connect"].push_back(result);
            txp->close(nullptr);
            done_cb(connect_error);
        };
    };

    std::vector<Continuation<Error>> continuations;
    for (auto ip_port : ip_ports) {
        std::list<std::string> ip_port_l = split(ip_port, ":");
        if (ip_port_l.size() != 2) {
            logger->warn("Couldn't split ip_port: %s", ip_port.c_str());
            (*entry)["failure"] = ValueError().reason;
            all_done_cb(ValueError());
            return;
        }
        std::string ip = ip_port_l.front();
        int port = std::stoi(ip_port_l.back());

        options["host"] = ip;
        options["port"] = port;
        continuations.push_back([=](Callback<Error> done_cb) {
            templates::tcp_connect(
                options, connected_cb(ip, port, done_cb), reactor, logger);
        });
    }
    mk::parallel(continuations, all_done_cb, 3 /* parallelism */);
}

static void http_many(const std::vector<std::string> urls, std::string type,
    SharedPtr<Entry> entry, Settings options, SharedPtr<Reactor> reactor,
    SharedPtr<Logger> logger, Callback<Error> all_done_cb) {
    if (type == "web") {
        // if any titles are not "Telegram Web", switch this to blocked
        (*entry)["telegram_web_status"] = "ok";
        (*entry)["telegram_web_failure"] = nullptr;
    }
    auto http_cb = [=](std::string url, Callback<Error> done_cb) {
        return [=](Error err, SharedPtr<http::Response> response) {
            if (!!err) {
                logger->info(
                    "telegram: failure HTTP connecting to %s", url.c_str());
                if (type == "web") {
                    (*entry)["telegram_web_status"] = "blocked";
                    (*entry)["telegram_web_failure"] = err.reason;
                }
            } else {
                logger->info(
                    "telegram: success HTTP connecting to %s", url.c_str());
                if (type == "endpoints") {
                    (*entry)["telegram_http_blocking"] = false;
                } else if (type == "web") {
                    if (extract_html_title(response->body) != "Telegram Web") {
                        (*entry)["telegram_web_status"] = "blocked";
                    }
                }
            }
            done_cb(err);
        };
    };

    std::vector<Continuation<Error>> continuations;
    for (auto url : urls) {
        options["http/url"] = url;
        http::Headers headers = constants::COMMON_CLIENT_HEADERS;
        std::string body;
        continuations.push_back([=](Callback<Error> done_cb) {
            templates::http_request(entry, options, headers, body,
                http_cb(url, done_cb), reactor, logger);
        });
    }
    mk::parallel(continuations, all_done_cb, 3 /* parallelism */);
}

void telegram(Settings options, Callback<SharedPtr<report::Entry>> callback,
    SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    std::vector<std::string> TELEGRAM_WEB_URLS = {
        "http://web.telegram.org/", "https://web.telegram.org/"};
    // should probably just make these std::pair<std::string,int>,
    // but I'm not sure if this will be better later when I get
    // rid of the duplication between this and the http ones
    std::vector<std::string> TELEGRAM_TCP_ENDPOINTS = {"149.154.175.50:80",
        "149.154.175.50:443", "149.154.167.51:80", "149.154.167.51:443",
        "149.154.175.100:80", "149.154.175.100:443", "149.154.167.91:80",
        "149.154.167.91:443", "149.154.171.5:80", "149.154.171.5:443"};

    // duplication for now to make things easier...
    std::vector<std::string> TELEGRAM_HTTP_ENDPOINTS = {
        "http://149.154.175.50:80", "http://149.154.175.50:443",
        "http://149.154.167.51:80", "http://149.154.167.51:443",
        "http://149.154.175.100:80", "http://149.154.175.100:443",
        "http://149.154.167.91:80", "http://149.154.167.91:443",
        "http://149.154.171.5:80", "http://149.154.171.5:443"};

    logger->info("starting telegram test");
    SharedPtr<Entry> entry(new Entry);

    // if any endpoints are (TCP or HTTP) reachable, switch this to false
    (*entry)["telegram_tcp_blocking"] = true;
    (*entry)["telegram_http_blocking"] = true;

    mk::fcompose(mk::fcompose_policy_async(),
        [=](Callback<> cb) {
            http_many(TELEGRAM_WEB_URLS, "web", entry, options, reactor, logger,
                [=](Error err) {
                    logger->info("saw %s in Telegram Web",
                        (!!err) ? "at least one error" : "no errors");
                    cb();
                });
        },
        [=](Callback<> cb) {
            tcp_many(TELEGRAM_TCP_ENDPOINTS, entry, options, reactor, logger,
                [=](Error err) {
                    logger->info("saw %s in Telegram's TCP endpoints",
                        (!!err) ? "at least one error" : "no errors");
                    cb();
                });
        },
        [=](Callback<> cb) {
            http_many(TELEGRAM_HTTP_ENDPOINTS, "endpoints", entry, options,
                reactor, logger, [=](Error err) {
                    logger->info("saw %s in Telegram's HTTP endpoints",
                        (!!err) ? "at least one error" : "no errors");
                    cb();
                });
        })([=]() {
        logger->info("calling final callback");
        callback(entry);
    });
}

} // namespace ooni
} // namespace mk
