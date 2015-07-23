// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/ooni.hpp>
#include <measurement_kit/common.hpp>

#include <iostream>

using namespace measurement_kit::common;
using namespace measurement_kit::ooni;

TEST_CASE("TCPTest test should run")
{
#if 0
    measurement_kit::set_verbose(1);

    auto client = TCPClient("www.torproject.org", "80");

    client.on("error", [](Error error) {
        std::cout << std::endl;  // Terminate eventual body
        if (error.error == 0) {
            std::cout << "Error: " << error.error << std::endl;
        }
        measurement_kit::break_loop();
    });

    client.on("connect", [&]() {
        std::cout << "Connection established..." << std::endl;

        client.write("GET /robots.txt HTTP/1.1\r\n"
                     "Connection: close\r\n"
                     "Host: www.neubot.org\r\n"
                     "Accept: */*\r\n"
                     "\r\n");

        client.on("flush", []() {
            std::cout << "Request sent... waiting for response" << std::endl;
        });

        client.on("data", [](std::string&& data) {
            std::cout << data;
        });
    });

    measurement_kit::loop();
#endif
}

TEST_CASE("TCPTest works as expected in a common case")
{
    measurement_kit::set_verbose(1);

    auto count = 0;
    TCPTest tcp_test("", Settings());

    auto client1 = tcp_test.connect({
        {"host", "www.neubot.org"},
        {"port", "80"},
    }, [&count]() {
        if (++count >= 2) {
            measurement_kit::break_loop();
        }
    });

    auto client2 = tcp_test.connect({
        {"host", "ooni.nu"},
        {"port", "80"},
    }, [&count]() {
        if (++count >= 2) {
            measurement_kit::break_loop();
        }
    });

    measurement_kit::loop();
}
