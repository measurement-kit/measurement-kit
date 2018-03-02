// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/ooni/constants.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"
#include "src/libmeasurement_kit/ooni/utils.hpp"
#include "src/libmeasurement_kit/ooni/templates.hpp"

#include <algorithm>
#include <cctype>
#include <set>

#define BODY_PROPORTION_FACTOR 0.7

namespace mk {
namespace ooni {

using namespace mk::report;

typedef std::vector<std::pair<std::string, int>> SocketList;

static void compare_http_requests(SharedPtr<Entry> entry,
                                  SharedPtr<http::Response> response, Entry control,
                                  SharedPtr<Logger> logger) {

    // The response may be null if HTTP fails due to network errors
    if (!response) {
        logger->warn("skip comparison due to null response");
        return;
    }

    std::string exp_body = response->body;
    int exp_length = exp_body.size();
    int ctrl_length = control["body_length"];

    // Verifiy if the body lengths match a certain proportion factor.
    float body_proportion = 0;
    if (ctrl_length == exp_length) {
        body_proportion = 1;
    } else if (ctrl_length == 0 || exp_length == 0) {
        body_proportion = 0;
    } else {
        body_proportion = (float)std::min(ctrl_length, exp_length) /
                          (float)std::max(ctrl_length, exp_length);
    }
    logger->debug("web_connectivity: body proportion %f", body_proportion);

    if (body_proportion > BODY_PROPORTION_FACTOR) {
        (*entry)["body_length_match"] = true;
    } else {
        (*entry)["body_length_match"] = false;
    }
    (*entry)["body_proportion"] = body_proportion;

    // Verify that the status codes match
    logger->debug("web_connectivity: comparing status codes");

    // We ignore status code matching when the server returns an error in the
    // control.
    (*entry)["status_code_match"] = true;
    if (((int)control["status_code"]) / 100 != 5) {
        if (((unsigned)control["status_code"]) == response->status_code) {
            (*entry)["status_code_match"] = true;
        } else {
            (*entry)["status_code_match"] = false;
        }
    }

    // Check if the headers match
    logger->debug("web_connectivity: checking headers");
    std::set<std::string> lowercase_ctrl_headers;
    std::set<std::string> lowercase_exp_headers;

    for (Entry::iterator it = control["headers"].begin();
         it != control["headers"].end(); ++it) {
        std::string lower_header(it.key());
        std::transform(lower_header.begin(),
                       lower_header.end(),
                       lower_header.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        lowercase_ctrl_headers.insert(lower_header);
    }
    for (auto it = response->headers.begin(); it != response->headers.end();
         ++it) {
        std::string lower_header(it->first);
        std::transform(lower_header.begin(),
                       lower_header.end(),
                       lower_header.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        lowercase_exp_headers.insert(lower_header);
    }

    if (lowercase_ctrl_headers == lowercase_exp_headers) {
        (*entry)["headers_match"] = true;
    } else {
        std::set<std::string> intersection;
        std::set<std::string> uncommon_intersection;

        std::set_intersection(
            lowercase_exp_headers.begin(), lowercase_exp_headers.end(),
            lowercase_ctrl_headers.begin(), lowercase_ctrl_headers.end(),
            std::inserter(intersection, intersection.begin()));

        std::set_difference(intersection.begin(), intersection.end(),
                            constants::COMMON_SERVER_HEADERS.begin(),
                            constants::COMMON_SERVER_HEADERS.end(),
                            std::inserter(uncommon_intersection,
                                          uncommon_intersection.begin()));

        if (uncommon_intersection.size() > 0) {
            (*entry)["headers_match"] = true;
        } else {
            (*entry)["headers_match"] = false;
        }
    }

    // Check if the HTML titles match
    logger->debug("web_connectivity: checking HTML titles");
    std::string experiment_title = extract_html_title(response->body);
    std::vector<std::string> exp_title_words;
    std::vector<std::string> ctrl_title_words;

    exp_title_words = split<std::vector<std::string>>(experiment_title, " ");
    ctrl_title_words = split<std::vector<std::string>>(control["title"], " ");
    size_t idx = 0;
    (*entry)["title_match"] = (experiment_title == control["title"]);
    for (auto exp_word : exp_title_words) {
        if (exp_word.length() < 5) {
            idx++;
            continue;
        }
        if (idx >= ctrl_title_words.size()) {
            break;
        }
        (*entry)["title_match"] = (bool)(exp_word == ctrl_title_words[idx]);
        break;
    }
}

static void compare_dns_queries(SharedPtr<Entry> entry,
                                std::vector<std::string> experiment_addresses,
                                Entry control, Settings options) {

    SharedPtr<Logger> logger = Logger::global();
    // When the controls fails in the same way as the experiment we consider the
    // DNS queries to be consistent.
    // XXX ensure the failure messages are aligned between ooniprobe and MK
    if ((*entry)["dns_experiment_failure"] != nullptr) {
        std::string exp_failure = (*entry)["dns_experiment_failure"];
        std::string ctrl_failure = control["failure"];
        if (exp_failure == ctrl_failure) {
            (*entry)["dns_consistency"] = "consistent";
        } else {
            (*entry)["dns_consistency"] = "inconsistent";
        }
        return;
    }

    std::set<std::string> exp_addresses(experiment_addresses.begin(),
                                        experiment_addresses.end());
    std::set<std::string> ctrl_addresses;
    for (std::string addr : control["addrs"]) {
        ctrl_addresses.insert(addr);
    }

    if (exp_addresses == ctrl_addresses) {
        (*entry)["dns_consistency"] = "consistent";
        return;
    }

    for (auto exp_addr : exp_addresses) {
        if (is_private_ipv4_addr(exp_addr) == true) {
            (*entry)["dns_consistency"] = "inconsistent";
            return;
        }
    }

    std::set<std::string> common_addresses;
    std::set_intersection(
        exp_addresses.begin(), exp_addresses.end(), ctrl_addresses.begin(),
        ctrl_addresses.end(),
        std::inserter(common_addresses, common_addresses.begin()));

    if (common_addresses.size() > 0) {
        (*entry)["dns_consistency"] = "consistent";
        return;
    }

    std::set<std::string> exp_asns;
    std::set<std::string> ctrl_asns;

    std::string asn_p = options.get("geoip_asn_path", std::string{});
    auto ip_location = GeoipCache::thread_local_instance()->get(asn_p);
    for (auto exp_addr : exp_addresses) {
        ErrorOr<std::string> asn = ip_location->resolve_asn(exp_addr);
        if (asn && asn.as_value() != "AS0") {
            exp_asns.insert(asn.as_value());
        }
    }
    for (auto ctrl_addr : ctrl_addresses) {
        ErrorOr<std::string> asn = ip_location->resolve_asn(ctrl_addr);
        if (asn && asn.as_value() != "AS0") {
            ctrl_asns.insert(asn.as_value());
        }
    }
    std::set<std::string> common_asns;
    std::set_intersection(exp_asns.begin(), exp_asns.end(), ctrl_asns.begin(),
                          ctrl_asns.end(),
                          std::inserter(common_asns, common_asns.begin()));

    if (common_asns.size() > 0) {
        (*entry)["dns_consistency"] = "consistent";
        return;
    }

    (*entry)["dns_consistency"] = "inconsistent";
}

static bool compare_tcp_connect(SharedPtr<Entry> entry, Entry control) {
    bool success = true;
    int idx = 0;
    for (auto result : (*entry)["tcp_connect"]) {
        bool ctrl_status;
        bool exp_status = result["status"]["success"];
        // XXX this is wasteful
        std::string ip = result["ip"];
        std::ostringstream ss;
        ss << ip;
        ss << ":";
        ss << result["port"];
        try {
            ctrl_status = control.at(ss.str())["status"];
        } catch (const std::out_of_range &) {
            continue;
        }
        if (ctrl_status == true && exp_status == false) {
            (*entry)["tcp_connect"][idx]["status"]["blocked"] = true;
            success = false;
        } else {
            (*entry)["tcp_connect"][idx]["status"]["blocked"] = false;
        }
        idx++;
    }
    return success;
}

static void compare_control_experiment(std::string input, SharedPtr<Entry> entry,
                                       SharedPtr<http::Response> response,
                                       std::vector<std::string> addresses,
                                       Settings options, SharedPtr<Logger> logger) {
    if ((*entry)["control_failure"] != nullptr) {
        logger->warn(
            "web_connectivity: skipping control comparison due to failure");
        return;
    }

    if ((*entry)["http_experiment_failure"] == nullptr &&
        (*entry)["control"]["http_request"]["failure"] == nullptr) {
        logger->debug("web_connectivity: comparing http_requests");
        compare_http_requests(entry, response,
                              (*entry)["control"]["http_request"], logger);
    }

    logger->debug("web_connectivity: comparing dns_queries");
    compare_dns_queries(entry, addresses, (*entry)["control"]["dns"], options);

    logger->debug("web_connectivity: comparing tcp_connect");
    bool tcp_connect_success =
        compare_tcp_connect(entry, (*entry)["control"]["tcp_connect"]);

    std::string exp_http_failure;
    std::string ctrl_http_failure;
    std::string dns_consistency = (*entry)["dns_consistency"];

    logger->debug("web_connectivity: exp,ctrl http failure determination");
    if ((*entry)["http_experiment_failure"] != nullptr) {
        exp_http_failure = split((*entry)["http_experiment_failure"]).front();
    }
    if ((*entry)["control"]["http_request"]["failure"] != nullptr) {
        ctrl_http_failure =
            split((*entry)["control"]["http_request"]["failure"]).front();
    }

    logger->debug("web_connectivity: checking if we got expected web page");

    bool got_expected_web_page = false;
    if (exp_http_failure == "" && ctrl_http_failure == "") {
        got_expected_web_page =
            ((((bool)(*entry)["body_length_match"]) == true ||
              ((bool)(*entry)["headers_match"]) == true ||
              ((bool)(*entry)["title_match"]) == true) &&
             ((bool)(*entry)["status_code_match"]) != false);
    }

    logger->debug("web_connectivity: determining blocking reason");

    if (dns_consistency == "consistent" && tcp_connect_success == false &&
        exp_http_failure != "") {
        (*entry)["blocking"] = "tcp_ip";
    } else if (dns_consistency == "consistent" && tcp_connect_success == true &&
               got_expected_web_page == false && exp_http_failure == "" &&
               ctrl_http_failure == "") {
        (*entry)["blocking"] = "http-diff";
    } else if (dns_consistency == "consistent" && tcp_connect_success == true &&
               exp_http_failure != "" && ctrl_http_failure == "") {
        if (exp_http_failure == "dns_lookup_error") {
            (*entry)["blocking"] = "dns";
        } else {
            (*entry)["blocking"] = "http-failure";
        }
    } else if (dns_consistency == "inconsistent" &&
               got_expected_web_page ==
                   false) { // Note this slightly differs from the OONI logic
        // because the got_expected_page is more strict in MK
        (*entry)["blocking"] = "dns";
    } else if (dns_consistency == "consistent" &&
               got_expected_web_page == false &&
               (exp_http_failure == "" && ctrl_http_failure == "") &&
               ((*entry)["control"]["dns"]["failure"] != nullptr ||
                ctrl_http_failure != exp_http_failure)) {
        (*entry)["blocking"] = "dns";
    }
    if ((*entry)["blocking"] != nullptr) {
        std::string blocking = (*entry)["blocking"];
        logger->info("web_connectivity: BLOCKING detected due to: %s on %s",
                     blocking.c_str(), input.c_str());
        (*entry)["accessible"] = false;
    } else {
        logger->info("web_connectivity: no blocking detected");
        (*entry)["accessible"] = true;
        (*entry)["blocking"] = false;
    }
}

static void control_request(http::Headers headers_to_pass_along,
                            SharedPtr<Entry> entry, SocketList socket_list,
                            std::string url, Callback<Error> callback,
                            Settings settings, SharedPtr<Reactor> reactor,
                            SharedPtr<Logger> logger) {

    // Implementation note: this function uses (and modifies) the settings
    // passed by the caller because such object is passed by copy and we
    // need to add options to a number of options already set by the caller
    // and it would not be wise to remember to copy them one by one (also
    // considering that from time to time we add new options)

    http::Headers headers;
    Entry request;
    request["tcp_connect"] = Entry::array();
    for (auto socket : socket_list) {
        // Formats the sockets as IP:PORT
        std::ostringstream ss;
        if (is_private_ipv4_addr(socket.first) == true) {
            continue;
        }
        ss << socket.first;
        ss << ":";
        ss << socket.second;
        request["tcp_connect"].push_back(ss.str());
    }
    request["http_request"] = url;
    // XXX in OONI headers are like `key: [value,...]` whereas in MK
    // they are like `key: value`. Adapt to OONI format.
    Entry true_headers;
    for (auto pair: headers_to_pass_along) {
        true_headers[pair.first].push_back(pair.second);
    }
    request["http_request_headers"] = true_headers;
    std::string body = request.dump();

    settings["http/url"] = settings["backend"];
    settings["http/method"] = "POST";
    headers["Content-Type"] = "application/json";

    if (settings["backend/type"] == "cloudfront") {
        // TODO set the appropriate headers to support cloud-fronting.
    }

    logger->info("Using backend %s", settings["backend"].c_str());
    logger->debug2("Body %s", body.c_str());

    mk::dump_settings(settings, "web_connectivity", logger);

    http::request(settings, headers, body,
                  [=](Error error, SharedPtr<http::Response> response) {
                      if (!error) {
                          try {
                              (*entry)["control"] =
                                  Entry::parse(response->body);
                              callback(NoError());
                              return;
                          } catch (const std::invalid_argument &) {
                              (*entry)["control_failure"] = "json_parse_error";
                              callback(JsonParseError());
                              return;
                          }
                      }
                      (*entry)["control_failure"] = error.reason;
                      callback(error);
                      return;
                  },
                  reactor, logger);
}

static void experiment_http_request(
        SharedPtr<Entry> entry, std::string url,
        Callback<Error, http::Headers, SharedPtr<http::Response>> cb,
        Settings options, SharedPtr<Reactor> reactor,
        SharedPtr<Logger> logger) {

    http::Headers headers = constants::COMMON_CLIENT_HEADERS;
    std::string body;
    options["http/url"] = url;

    /*
     * Only for web-connectivity:
     *
     * - we want to allow any SSL protocol such that we can scan a
     *   more wide range of servers
     *
     * - we allow SSL dirty shutdowns to gather more evidence
     */
    options["net/allow_ssl23"] = true;
    options["net/ssl_allow_dirty_shutdown"] = true;

    logger->debug("Requesting url %s", url.c_str());
    templates::http_request(entry, options, headers, body,
                            [=](Error err, SharedPtr<http::Response> response) {
                                if (err) {
                                    (*entry)["http_experiment_failure"] =
                                        err.reason;
                                    cb(err, headers, response);
                                    return;
                                }
                                cb(NoError(), headers, response);
                            },
                            reactor, logger);
}

static void experiment_tcp_connect(SharedPtr<Entry> entry, SocketList sockets,
                                   Callback<Error> cb, SharedPtr<Reactor> reactor,
                                   SharedPtr<Logger> logger) {

    int socket_count = sockets.size();
    SharedPtr<int> sockets_tested(new int(0));
    // XXX this is very ghetto
    if (socket_count == 0) {
        cb(NoError());
        return;
    }

    auto handle_connect = [=](std::string ip, int port) {
        return [=](Error err, SharedPtr<net::Transport> txp) {
            *sockets_tested += 1;
            bool close_txp = true;
            Entry result = {
                {"ip", ip},
                {"port", port},
                {"status",
                 {{"success", nullptr},
                  {"failure", nullptr},
                  {"blocked", nullptr}}},
            };

            if (!!err) {
                logger->info("web_connectivity: failed to connect to %s:%d",
                             ip.c_str(), port);
                result["status"]["success"] = false;
                result["status"]["failure"] = err.reason;
                close_txp = false;
            } else {
                logger->info("web_connectivity: success to connect to %s:%d",
                             ip.c_str(), port);
                result["status"]["success"] = true;
                result["status"]["blocked"] = false;
            }
            (*entry)["tcp_connect"].push_back(result);
            if (socket_count == *sockets_tested) {
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

    for (auto socket : sockets) {
        std::string address = socket.first;
        int port = socket.second;
        Settings connect_options;
        connect_options["host"] = address;
        connect_options["port"] = port;
        connect_options["net/timeout"] = 10.0;
        templates::tcp_connect(connect_options, handle_connect(address, port),
                               reactor, logger);
    }
}

static void experiment_dns_query(
    SharedPtr<Entry> entry, std::string hostname, std::string nameserver,
    Callback<Error, std::vector<std::string>> callback, Settings options,
    SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {

    if (net::is_ip_addr(hostname)) {
        // Don't perform DNS resolutions if it's an IP address
        // XXX This means we are not filling the entry
        std::vector<std::string> addresses;
        addresses.push_back(hostname);
        callback(NoError(), addresses);
        return;
    }

    templates::dns_query(
        entry, "A", "IN", hostname, nameserver,
        [=](Error err, SharedPtr<dns::Message> message) {
            std::vector<std::string> addresses;
            if (err) {
                callback(err, addresses);
                return;
            }
            for (auto answer : message->answers) {
                if (answer.ipv4 != "") {
                    addresses.push_back(answer.ipv4);
                } else {
                    /* Not yet implemented */ ;
                }
            }
            callback(NoError(), addresses);
        },
        options, reactor, logger);
}

void web_connectivity(std::string input, Settings options,
                      Callback<SharedPtr<Entry>> callback, SharedPtr<Reactor> reactor,
                      SharedPtr<Logger> logger) {
    options["http/max_redirects"] = 20;
    SharedPtr<Entry> entry(new Entry);
    // This is set from ooni test
    // (*entry)["client_resolver"] = nullptr;
    (*entry)["retries"] = nullptr;

    (*entry)["dns_consistency"] = nullptr;
    (*entry)["body_length_match"] = nullptr;
    (*entry)["body_proportion"] = 0.0;
    (*entry)["headers_match"] = nullptr;
    (*entry)["status_code_match"] = nullptr;
    (*entry)["title_match"] = nullptr;

    (*entry)["accessible"] = nullptr;
    (*entry)["blocking"] = nullptr;

    (*entry)["control_failure"] = nullptr;
    (*entry)["http_experiment_failure"] = nullptr;
    (*entry)["dns_experiment_failure"] = nullptr;

    (*entry)["tcp_connect"] = Entry::array();
    (*entry)["control"] = Entry({});

    if (!mk::startswith(input, "http://") &&
        !mk::startswith(input, "https://")) {
        // Similarly to ooni-probe also accept a list of endpoints
        input = "http://" + input;
    }

    ErrorOr<http::Url> url = mk::http::parse_url_noexcept(input);

    if (!url) {
        logger->warn("Invalid test url.");
        (*entry)["failure"] = url.as_error().reason;
        callback(entry);
        return;
    }

    std::string hostname = url->address;
    std::string nameserver = options["nameserver"];
    if (nameserver != "") {
        logger->warn("web_connectivity: you're using the deprecated "
                     "'nameserver' option");
    }

    logger->info("web_connectivity: starting dns_query for %s",
                 hostname.c_str());

    // TODO: is this correct here to continue on errors?

    experiment_dns_query(
        entry, hostname, nameserver,
        [=](Error err, std::vector<std::string> addresses) {

            if (err) {
                logger->warn("web_connectivity: dns-query error: %s",
                             err.what());
            }
            logger->info("web_connectivity: starting tcp_connect");

            SocketList socket_list;
            for (auto addr : addresses) {
                socket_list.push_back(std::make_pair(addr, url->port));
            }

            experiment_tcp_connect(
                entry, socket_list,
                [=](Error err) {

                    if (err) {
                        logger->warn("web_connectivity: tcp-connect error: %s",
                                     err.what());
                    }

                    logger->info(
                        "web_connectivity: starting http_request to %s",
                        input.c_str());
                    experiment_http_request(
                        entry, input,
                        [=](Error err, http::Headers request_headers,
                            SharedPtr<http::Response> response) {

                            if (err) {
                                logger->warn(
                                    "web_connectivity: http-request error: %s",
                                    err.what());
                            }

                            logger->info(
                                "web_connectivity: doing control request");
                            control_request(
                                request_headers, entry,
                                socket_list, input,
                                [=](Error err) {

                                    if (err) {
                                        logger->warn("web_connectivity: "
                                                     "control-request error: "
                                                     "%s",
                                                     err.what());
                                    }

                                    logger->info("web_connectivity: comparing "
                                                 "control with experiment");
                                    compare_control_experiment(
                                        input, entry, response, addresses,
                                        options, logger);
                                    callback(entry);

                                },
                                options, reactor,
                                logger); // end control_request

                        },
                        options, reactor, logger); // end http_request

                },
                reactor, logger); // end tcp_connect

        },
        options, reactor, logger); // end dns_query
}

} // namespace ooni
} // namespace mk
