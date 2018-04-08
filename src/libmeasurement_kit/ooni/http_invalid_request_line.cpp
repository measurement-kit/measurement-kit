// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"
#include "src/libmeasurement_kit/ooni/templates.hpp"
#include "src/libmeasurement_kit/report/entry.hpp"
#include "src/libmeasurement_kit/net/transport.hpp"

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

static const int timeout = 5;

static void send_receive_invalid_request_line(net::Endpoint endpoint,
                                              std::string request_line,
                                              Callback<SharedPtr<report::Entry>> cb,
                                              Settings settings,
                                              SharedPtr<Reactor> reactor,
                                              SharedPtr<Logger> logger) {
    settings["host"] = endpoint.hostname;
    settings["port"] = endpoint.port;
    SharedPtr<report::Entry> entry{new report::Entry{
        {"tampering", nullptr},
        {"received", nullptr},
        {"sent", nullptr},
        {"failure", nullptr}
    }};
    templates::tcp_connect(settings, [=](Error err, SharedPtr<net::Transport> txp) {
        if (err) {
            logger->warn("http_invalid_request_line: error connecting");
            (*entry)["failure"] = err.reason;
            cb(entry);
            return;
        }
        SharedPtr<std::string> received_data(new std::string);
        txp->on_data([=](net::Buffer data) {
            logger->debug("http_invalid_request_line: on_data: %s",
                          data.peek().c_str());
            *received_data += data.read();
        });
        txp->write(request_line);

        // We assume to have received all the data after a timeout
        // of 5 seconds.
        reactor->call_later(timeout, [=]() {
            if (*received_data != request_line) {
                logger->warn("Tampering detected: '%s' != '%s'",
                             received_data->c_str(), request_line.c_str());
                (*entry)["tampering"] = true;
            } else {
                logger->debug("Tampering not detected: '%s' == '%s'",
                              received_data->c_str(), request_line.c_str());
                (*entry)["tampering"] = false;
            }
            (*entry)["sent"] = request_line;
            (*entry)["received"] = *received_data;
            txp->close([=]() { cb(entry); });
        });
    }, reactor, logger);
}

void http_invalid_request_line(Settings options,
                               Callback<SharedPtr<report::Entry>> cb,
                               SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    SharedPtr<report::Entry> entry(new report::Entry);
    (*entry)["tampering"] = nullptr;
    (*entry)["received"] = report::Entry::array();
    (*entry)["sent"] = report::Entry::array();
    (*entry)["tampering_list"] = report::Entry::array();
    (*entry)["failure_list"] = report::Entry::array();
    SharedPtr<int> tests_run(new int(0));

    ErrorOr<net::Endpoint> endpoint =
        mk::net::parse_endpoint(options["backend"], 80);

    if (!endpoint) {
        logger->warn("Invalid helper endpoint: %s (backend = '%s')",
                     endpoint.as_error().what(),
                     options["backend"].c_str());
        (*entry)["failure"] = endpoint.as_error().reason;
        cb(entry);
        return;
    }

    logger->info("Using helper: %s", options["backend"].c_str());

    auto handle_response = [=](SharedPtr<report::Entry> inner) {
        *tests_run += 1;
        (*entry)["tampering_list"].push_back((*inner)["tampering"]);
        (*entry)["received"].push_back((*inner)["received"]);
        (*entry)["sent"].push_back((*inner)["sent"]);
        (*entry)["failure_list"].push_back((*inner)["failure"]);
        if (*tests_run == 4) {
            for (auto &x : (*entry)["failure_list"]) {
                if (x != nullptr) {
                    (*entry)["failure"]
                        = ParallelOperationError().reason;
                    break;
                }
            }
            for (auto &x : (*entry)["tampering_list"]) {
                if (x != nullptr) {
                    (*entry)["tampering"] = x;
                    if ((*entry)["tampering"] == true) {
                        break;
                    }
                }
            }
            cb(entry);
        }
    };

    // test_random_invalid_method
    // randomSTR(4) + " / HTTP/1.1\n\r"
    std::string test_random_invalid_method(mk::random_str_uppercase(4));
    test_random_invalid_method += " / HTTP/1.1\n\r";
    send_receive_invalid_request_line(
        *endpoint, test_random_invalid_method, handle_response,
        options, reactor, logger);

    // test_random_invalid_field_count
    // ' '.join(randomStr(5) for x in range(4)) + '\n\r'
    std::string test_random_invalid_field_count(mk::random_str_uppercase(5));
    for (int i = 0; i < 3; i++) {
        test_random_invalid_field_count += " " + mk::random_str_uppercase(5);
    }
    test_random_invalid_field_count += "\n\r";
    send_receive_invalid_request_line(
        *endpoint, test_random_invalid_field_count, handle_response,
        options, reactor, logger);

    // test_random_big_request_method
    // randomStr(1024) + ' / HTTP/1.1\n\r'
    std::string test_random_big_request_method(mk::random_str_uppercase(1024));
    test_random_big_request_method += " / HTTP/1.1\n\r";
    send_receive_invalid_request_line(
        *endpoint, test_random_big_request_method, handle_response,
        options, reactor, logger);

    // test_random_invalid_version_number
    // 'GET / HTTP/' + randomStr(3)
    std::string test_random_invalid_version_number("GET / HTTP/");
    test_random_invalid_version_number += mk::random_str_uppercase(3);
    send_receive_invalid_request_line(
        *endpoint, test_random_invalid_version_number,
        handle_response, options, reactor, logger);

    // test_squid_cache_manager
    // 'GET cache_object://localhost/ HTTP/1.0\n\r'
    std::string test_squid_cache_manager("GET cache_object://localhost/ HTTP/1.0\n\r");
    send_receive_invalid_request_line(
      *endpoint, test_squid_cache_manager, handle_response,
      options, reactor, logger);
}

} // namespace ooni
} // namespace mk
