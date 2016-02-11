// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_BEV_CONNECT_HPP
#define SRC_NET_BEV_CONNECT_HPP

#include <functional>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/logger.hpp>
#include <string>
#include <vector>

// Forward declarations
struct sockaddr;
struct bufferevent;

namespace mk {

// Forward declarations
class Error;
template <typename T> class Var;

namespace net {

typedef std::function<void(Error, Var<bufferevent>)> OnConnect;

void connect(sockaddr *sa, int len, OnConnect callback, double timeout = -1.0,
             Poller * = Poller::global(), Logger * = Logger::global());

void connect(std::string endpoint, OnConnect callback, double timeout = -1.0,
             Poller * = Poller::global(), Logger * = Logger::global());

void connect_one_of(std::vector<std::string> endpoints, OnConnect callback,
                    double timeout = -1.0, Poller *poller = Poller::global(),
                    Logger * = Logger::global());

Error __attribute__((warn_unused_result))
connect_dns_sync(std::string addr, std::string port, OnConnect, double = -1.0,
                 Poller * = Poller::global(), Logger * = Logger::global());

Error __attribute__((warn_unused_result))
connect_dns_sync(std::string addr, int, OnConnect cb, double timeout = -1.0,
                 Poller * = Poller::global(), Logger * = Logger::global());

void connect_ssl(Var<bufferevent> bev, OnConnect cb, double timeout = -1.0,
                 Poller * = Poller::global(), Logger * = Logger::global());

} // namespace net
} // namespace mk
#endif
