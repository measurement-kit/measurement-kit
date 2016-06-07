// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_HTTP_INVALID_REQUEST_LINE_HPP
#define SRC_OONI_HTTP_INVALID_REQUEST_LINE_HPP

#include <measurement_kit/http.hpp>
#include <measurement_kit/ooni.hpp>

#include "src/common/utils.hpp"
#include "src/ooni/ooni_test_impl.hpp"
#include "src/ooni/tcp_test_impl.hpp"
#include <sys/stat.h>

namespace mk {
namespace ooni {

class HTTPInvalidRequestLineImpl : public TCPTestImpl {
    using TCPTestImpl::TCPTestImpl;

    int tests_run = 0;
    // Timeout in seconds after which we consider all the data received.
    int timeout = 5;

  public:
    HTTPInvalidRequestLineImpl(Settings options_) : TCPTestImpl(options_) {
        test_name = "http_invalid_request_line";
        test_version = "0.0.1";
    }

    void send_receive_invalid_request_line(Var<report::Entry> entry,
                                           http::Url backend_url,
                                           std::string request_line,
                                           std::function<void()> &&cb,
                                           Settings settings = {}) {
        settings["host"] = backend_url.address;
        settings["port"] = backend_url.port;
        connect(settings,
                [=](Error err, Var<net::Transport> txp) {
                    if (err) {
                        logger->debug("http_invalid_request_line: error connecting");
                        cb();
                        return;
                    }
                    Var<std::string> received_data(new std::string);
                    txp->on_data([this, received_data](net::Buffer data) {
                        logger->debug("http_invalid_request_line: on_data");
                        *received_data += data.read();
                        logger->debug("%s", received_data->c_str());
                    });
                    txp->write(request_line);

                    // We assume to have received all the data after a timeout
                    // of 5 seconds.
                    reactor->call_later(timeout, [=]() {
                        if (*received_data != request_line) {
                            logger->info("Tampering detected!");
                            logger->info("%s != %s", received_data->c_str(),
                                         request_line.c_str());
                            (*entry)["tampering"] = true;
                        } else if ((*entry)["tampering"] == nullptr) {
                            logger->info("Tampering not detected.");
                            (*entry)["tampering"] = false;
                        }
                        (*entry)["sent"].push_back(request_line);
                        (*entry)["received"].push_back(*received_data);
                        txp->close([this, cb]() { cb(); });
                    });
                });
    }

    void main(Settings options, std::function<void(report::Entry)> &&cb) {
        Var<report::Entry> entry(new report::Entry);
        (*entry)["tampering"] = nullptr;
        (*entry)["received"] = report::Entry::array();
        (*entry)["sent"] = report::Entry::array();

        ErrorOr<http::Url> backend_url =
            mk::http::parse_url_noexcept(options["backend"]);

        if (!backend_url) {
            logger->debug("Invalid backend url.");
            cb(*entry);
            return;
        }

        auto handle_response = [=]() {
            tests_run += 1;
            if (tests_run == 4) {
                cb(*entry);
            }
        };

        // test_random_invalid_method
        // randomSTR(4) + " / HTTP/1.1\n\r"
        std::string test_random_invalid_method(mk::random_str_uppercase(4));
        test_random_invalid_method += " / HTTP/1.1\n\r";
        send_receive_invalid_request_line(entry, *backend_url,
                test_random_invalid_method, handle_response, options);

        // test_random_invalid_field_count
        // ' '.join(randomStr(5) for x in range(4)) + '\n\r'
        std::string test_random_invalid_field_count(
            mk::random_str_uppercase(5));
        for (int i = 0; i < 3; i++) {
            test_random_invalid_field_count +=
                " " + mk::random_str_uppercase(5);
        }
        test_random_invalid_field_count += "\n\r";
        send_receive_invalid_request_line(entry, *backend_url,
                                          test_random_invalid_field_count,
                                          handle_response, options);

        // test_random_big_request_method
        // randomStr(1024) + ' / HTTP/1.1\n\r'
        std::string test_random_big_request_method(
            mk::random_str_uppercase(1024));
        test_random_big_request_method += " / HTTP/1.1\n\r";
        send_receive_invalid_request_line(entry, *backend_url,
                                          test_random_big_request_method,
                                          handle_response, options);

        // test_random_invalid_version_number
        // 'GET / HTTP/' + randomStr(3)
        std::string test_random_invalid_version_number("GET / HTTP/");
        test_random_invalid_version_number += mk::random_str_uppercase(3);
        send_receive_invalid_request_line(entry, *backend_url,
                                          test_random_invalid_version_number,
                                          handle_response, options);
    }
};

} // namespace ooni
} // namespace mk
#endif
