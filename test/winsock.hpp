// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef TEST_WINSOCK_HPP
#define TEST_WINSOCK_HPP
#ifdef _WIN32

#include <winsock2.h>
#include <iostream>
#include <stdlib.h>

class WinsockInit {
  public:
    WinsockInit() {
        WORD requested = MAKEWORD(2, 2);
        WSADATA data;
        if (::WSAStartup(requested, &data) != 0) {
            std::clog << "fatal: WSAStartup() failed" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
};

static const WinsockInit &wsainit_ = WinsockInit();

#endif
#endif
