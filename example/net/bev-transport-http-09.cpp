// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <functional>
#include <iostream>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/var.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include "src/net/bev-connect.hpp"
#include "src/net/bev-protocols.hpp"
#include "src/net/bev-transport.hpp"

// Forward declaration
struct bufferevent;

using namespace mk;

#define USAGE "usage: %s [-Sv] [-A address] [-p port] [path]\n"

static void do_http_09(Poller *poller, Var<bufferevent> bev, std::string path,
                       std::string *outp) {
    std::clog << "sending request and waiting for response...";
    auto tbev = net::TransportBev::create(bev, poller);
    net::http_09_sendrecv(tbev, "GET " + path + " \r\n", [outp, poller](Error error) {
        if (error) {
            std::clog << "error: " << error << "\n";
            poller->break_loop();
            return;
        }
        std::clog << "ok\n";
        std::cout << *outp << "\n";
        poller->break_loop();
    }, outp, 3.0);
}

int main(int argc, char **argv) {
    std::string address = "130.192.181.193";
    std::string path = "/";
    std::string port = "80";
    const char *progname = argv[0];
    bool use_ssl = false;
    int ch;

    while ((ch = getopt(argc, argv, "A:p:Sv")) != -1) {
        switch (ch) {
        case 'A':
            address = optarg;
            break;
        case 'p':
            port = optarg;
            break;
        case 'S':
            use_ssl = true;
            break;
        case 'v':
            mk::set_verbose(1);
            break;
        default:
            fprintf(stderr, USAGE, progname);
            exit(1);
        }
    }
    argc -= optind, argv += optind;
    if (argc != 0 && argc != 1) {
        fprintf(stderr, USAGE, progname);
        exit(1);
    }
    if (argc == 1) path = argv[0];

    Poller *poller = Poller::global();
    std::string out;
    std::string *outp = &out;
    poller->call_soon([address, outp, path, poller, port, use_ssl]() {
        Error err = net::connect_dns_sync(
            address, port,
            [outp, path, poller, use_ssl](Error err, Var<bufferevent> bev) {
                if (err) {
                    poller->break_loop();
                    return;
                }
                if (!use_ssl) {
                    do_http_09(poller, bev, path, outp);
                    return;
                }
                net::connect_ssl(
                    bev,
                    [outp, path, poller](Error err, Var<bufferevent> bev) {
                        if (err) {
                            poller->break_loop();
                            return;
                        }
                        do_http_09(poller, bev, path, outp);
                    });
            },
            3.0, poller);
        if (err) {
            poller->break_loop();
            return;
        }
    });
    poller->loop();
}
