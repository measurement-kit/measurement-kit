// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/common.hpp>

static void begin(mk::SharedPtr<mk::Socket> sock) {
    sock->read([sock](mk::Error err, mk::SharedPtr<mk::SocketBuffer> buff) {
        if (err) {
            std::clog << err << "\n";
            sock->close();
            return;
        }
        sock->write(buff, [sock](mk::Error err) {
            if (err) {
                std::clog << err << "\n";
                sock->close();
                return;
            }
            begin(sock);
        });
    });
}

int main() {
    auto reactor = mk::Reactor::make();
    auto logger = mk::Logger::make();
    logger->set_verbosity(MK_LOG_DEBUG2);
    // auto sock = mk::Socket::attach(0, reactor, logger);
    reactor->run_with_initial_event([=]() {
        // begin(sock);
        sockaddr_in sin{};
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = inet_addr("128.0.0.1");
        sin.sin_port = htons(54321);
        mk::Socket::connect((sockaddr *)&sin, sizeof(sin), {}, reactor, logger,
                [](mk::Error err, mk::SharedPtr<mk::Socket> sock) {
                    if (err) {
                        std::clog << err << "\n";
                        return;
                    }
                    begin(sock);
                });
    });
}
