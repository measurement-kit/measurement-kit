// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_SOCKET_HPP
#define MEASUREMENT_KIT_COMMON_SOCKET_HPP

#include <cstdint>
#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/portable/sys/socket.h>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/shared_ptr.hpp>
#include <string>

struct evbuffer; // Forward declaration to avoid pulling in libevent explicitly

namespace mk {

class Reactor; // Forward declaration to avoid circular dependency

#ifdef _WIN32
using socket_t = uintptr_t;
#elif DOXYGEN
/// \brief `socket_t` is a type suitable to contain a system socket.
using socket_t = platform_dependent;
#else
using socket_t = int;
#endif

class SocketBuffer {
  public:
    static SharedPtr<SocketBuffer> make();

    static SharedPtr<SocketBuffer> make(std::string &&data);

    static SharedPtr<SocketBuffer> make(evbuffer *data);

    virtual evbuffer *as_evbuffer() = 0;

    virtual size_t size() = 0;

    virtual const void *data() = 0;

    virtual ~SocketBuffer();
};

class Socket {
  public:
    static SharedPtr<Socket> attach(socket_t sockfd, SharedPtr<Reactor> reactor,
            SharedPtr<Logger> logger);

    static void connect(sockaddr *sa, socklen_t salen, Settings settings,
            SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
            Callback<Error, SharedPtr<Socket>> &&callback);

    virtual void set_timeout(double) = 0;

    virtual long long fd() = 0;

    virtual void read(Callback<Error, SharedPtr<SocketBuffer>> &&cb) = 0;

    virtual void write(SharedPtr<SocketBuffer> buff, Callback<Error> &&cb) = 0;

    virtual void close() = 0;

    virtual SharedPtr<Reactor> reactor() = 0;

    virtual SharedPtr<Logger> logger() = 0;

    virtual ~Socket();
};

} // namespace mk
#endif
