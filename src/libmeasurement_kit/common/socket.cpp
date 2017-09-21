// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/common/libevent/socket.hpp>
#include <string.h>

namespace mk {

/*static*/ SharedPtr<SocketBuffer> SocketBuffer::make() {
    return SharedPtr<SocketBuffer>{std::make_shared<LibeventSocketBuffer<>>()};
}

/*static*/ SharedPtr<SocketBuffer> SocketBuffer::make(std::string &&data) {
    return SharedPtr<SocketBuffer>{
            std::make_shared<LibeventSocketBuffer<>>(std::move(data))};
}

/*static*/ SharedPtr<SocketBuffer> SocketBuffer::make(evbuffer *data) {
    return SharedPtr<SocketBuffer>{
            std::make_shared<LibeventSocketBuffer<>>(data)};
}

SocketBuffer::~SocketBuffer() {}

/*static*/ SharedPtr<Socket> Socket::attach(
        socket_t sockfd, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    return LibeventSocket<>::attach(sockfd, reactor, logger);
}

/*static*/ void Socket::connect(sockaddr *sa, socklen_t salen,
        Settings settings, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
        Callback<Error, SharedPtr<Socket>> &&callback) {
    sockaddr_storage ss{};
    memcpy(&ss, sa, salen);
    reactor->call_soon([ =, callback = std::move(callback) ]() mutable {
        LibeventSocket<>::connect((sockaddr *)&ss, salen, settings, reactor,
                logger, std::move(callback));
    });
}

Socket::~Socket() {}

} // namespace mk
