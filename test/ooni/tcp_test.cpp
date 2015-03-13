
#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/ooni/tcp_test.hpp>
#include <ight/common/poller.hpp>
#include <ight/common/log.hpp>
#include <ight/common/utils.hpp>

#include <iostream>

using namespace ight::ooni::tcp_test;

TEST_CASE("TCPTest test should run")
{
#if 0
    ight_set_verbose(1);

    auto client = TCPClient("www.torproject.org", "80");

    client.on("error", [](Error error) {
        std::cout << std::endl;  // Terminate eventual body
        if (error.error == 0) {
            std::cout << "Error: " << error.error << std::endl;
        }
        ight_break_loop();
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

    ight_loop();
#endif
}

TEST_CASE("TCPTest works as expected in a common case")
{
    ight_set_verbose(1);

    auto count = 0;
    TCPTest tcp_test("", ight::common::Settings());

    auto client1 = tcp_test.connect({
        {"host", "www.neubot.org"},
        {"port", "80"},
    }, [&count]() {
        if (++count >= 2) {
            ight_break_loop();
        }
    });

    auto client2 = tcp_test.connect({
        {"host", "ooni.nu"},
        {"port", "80"},
    }, [&count]() {
        if (++count >= 2) {
            ight_break_loop();
        }
    });

    ight_loop();
}
