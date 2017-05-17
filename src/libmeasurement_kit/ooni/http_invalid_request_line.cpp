// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

static const int timeout = 5;

static void send_receive_invalid_request_line(http::Url backend_url,
                                              std::string request_line,
                                              Callback<Var<report::Entry>> cb,
                                              Settings settings,
                                              Var<Reactor> reactor,
                                              Var<Logger> logger) {
    settings["host"] = backend_url.address;
    settings["port"] = backend_url.port;
    Var<report::Entry> entry{new report::Entry{
        {"tampering", nullptr},
        {"received", nullptr},
        {"sent", nullptr},
        {"failure", nullptr}
    }};
    templates::tcp_connect(settings, [=](Error err, Var<net::Transport> txp) {
        if (err) {
            logger->warn("http_invalid_request_line: error connecting");
            (*entry)["failure"] = err.as_ooni_error();
            cb(entry);
            return;
        }
        Var<std::string> received_data(new std::string);
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
                               Callback<Var<report::Entry>> cb,
                               Var<Reactor> reactor, Var<Logger> logger) {
    Var<report::Entry> entry(new report::Entry);
    (*entry)["tampering"] = nullptr;
    (*entry)["received"] = report::Entry::array();
    (*entry)["sent"] = report::Entry::array();
    (*entry)["tampering_list"] = report::Entry::array();
    (*entry)["failure_list"] = report::Entry::array();
    Var<int> tests_run(new int(0));

    ErrorOr<http::Url> backend_url =
        mk::http::parse_url_noexcept(options["backend"]);

    if (!backend_url) {
        logger->debug("Invalid backend url.");
        (*entry)["failure"] = backend_url.as_error().as_ooni_error();
        cb(entry);
        return;
    }

    auto handle_response = [=](Var<report::Entry> inner) {
        *tests_run += 1;
        (*entry)["tampering_list"].push_back((*inner)["tampering"]);
        (*entry)["received"].push_back((*inner)["received"]);
        (*entry)["sent"].push_back((*inner)["sent"]);
        (*entry)["failure_list"].push_back((*inner)["failure"]);
        if (*tests_run == 4) {
            for (auto &x : (*entry)["failure_list"]) {
                if (x != nullptr) {
                    (*entry)["failure"]
                        = ParallelOperationError().as_ooni_error();
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
        *backend_url, test_random_invalid_method, handle_response,
        options, reactor, logger);

    // test_random_invalid_field_count
    // ' '.join(randomStr(5) for x in range(4)) + '\n\r'
    std::string test_random_invalid_field_count(mk::random_str_uppercase(5));
    for (int i = 0; i < 3; i++) {
        test_random_invalid_field_count += " " + mk::random_str_uppercase(5);
    }
    test_random_invalid_field_count += "\n\r";
    send_receive_invalid_request_line(
        *backend_url, test_random_invalid_field_count, handle_response,
        options, reactor, logger);

    // test_random_big_request_method
    // randomStr(1024) + ' / HTTP/1.1\n\r'
    std::string test_random_big_request_method(mk::random_str_uppercase(1024));
    test_random_big_request_method += " / HTTP/1.1\n\r";
    send_receive_invalid_request_line(
        *backend_url, test_random_big_request_method, handle_response,
        options, reactor, logger);

    // test_random_invalid_version_number
    // 'GET / HTTP/' + randomStr(3)
    std::string test_random_invalid_version_number("GET / HTTP/");
    test_random_invalid_version_number += mk::random_str_uppercase(3);
    send_receive_invalid_request_line(
        *backend_url, test_random_invalid_version_number,
        handle_response, options, reactor, logger);
}

} // namespace ooni
} // namespace mk
