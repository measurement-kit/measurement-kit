// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../src/libmeasurement_kit/dns/sendrecv_impl.hpp"

using namespace mk;

static socket_t
net_socket_create_fail(int, int, int, Var<Logger>) {
    return -1;
}

TEST_CASE("dns::sendrecv() handles net_socket_create() error") {
    addrinfo hints = {};
    ErrorOr<Var<socket_t>> maybe_sock =
        dns::send_impl<net_socket_create_fail>(
            "8.8.8.8", "53", "invalid", Logger::global()
        );
    REQUIRE(!maybe_sock);
    REQUIRE(maybe_sock.as_error().code == dns::SocketCreateError().code);
}

static int setsockopt_fail(socket_t, int, int, const void *, socklen_t) {
    return -1;
}

TEST_CASE("dns::sendrecv() handles setsockopt() error") {
    addrinfo hints = {};
    ErrorOr<Var<socket_t>> maybe_sock =
        dns::send_impl<net::socket_create, setsockopt_fail>(
            "8.8.8.8", "53", "invalid", Logger::global()
        );
    REQUIRE(!maybe_sock);
    REQUIRE(maybe_sock.as_error().code == dns::SetsockoptError().code);
}

static ErrorOr<Var<addrinfo>>
getaddrinfo_numeric_datagram_fail(const char *, const char *, Var<Logger>) {
    return MockedError();
}

TEST_CASE("dns::sendrecv() handles getaddrinfo_numeric_datagram() error") {
    addrinfo hints = {};
    ErrorOr<Var<socket_t>> maybe_sock =
        dns::send_impl<net::socket_create, setsockopt,
            getaddrinfo_numeric_datagram_fail>(
                "8.8.8.8", "53", "invalid", Logger::global()
            );
    REQUIRE(!maybe_sock);
    REQUIRE(maybe_sock.as_error().code == MockedError().code);
}

static ssize_t sendto_fail(socket_t, const void *, size_t, int,
                           const sockaddr *, socklen_t) {
    return -1;
}

TEST_CASE("dns::sendrecv() handles sendto() error") {
    addrinfo hints = {};
    ErrorOr<Var<socket_t>> maybe_sock =
        dns::send_impl<net::socket_create, setsockopt,
            dns::getaddrinfo_numeric_datagram, sendto_fail>(
                "8.8.8.8", "53", "invalid", Logger::global()
            );
    REQUIRE(!maybe_sock);
    REQUIRE(maybe_sock.as_error().code == dns::SendtoError().code);
}

static ssize_t sendto_truncate(socket_t, const void *, size_t, int,
                               const sockaddr *, socklen_t) {
    return 3 /* Different from what is passed as argument */;
}

TEST_CASE("dns::sendrecv() handles sendto() truncating packets") {
    addrinfo hints = {};
    ErrorOr<Var<socket_t>> maybe_sock =
        dns::send_impl<net::socket_create, setsockopt,
            dns::getaddrinfo_numeric_datagram, sendto_truncate>(
                "8.8.8.8", "53", "invalid", Logger::global()
            );
    REQUIRE(!maybe_sock);
    REQUIRE(maybe_sock.as_error().code == dns::PacketTruncatedError().code);
}

#ifdef ENABLE_INTEGRATION_TESTS
static bool close_called = false;
static int close_mock(socket_t sop) {
    close_called = true;
    evutil_closesocket(sop);
    return 0;
}

/*
 * Not a _true_ memory leak test. Still useful when I run `make check` on
 * my macOS system where there is no Valgrind. At least, I know that the
 * way in which `getaddrinfo` works is such that it arranges for `freeaddrinfo`
 * to be called when the returned value goes out of scope.
 *
 * When there is a reliable Valgrind on macOS, we can consider removing this
 * regress test, but until then I'd rather keep it.
 */
TEST_CASE("Make sure close() is called") {
    {
        addrinfo hints = {};
        ErrorOr<Var<socket_t>> maybe_sock =
            dns::send_impl<net::socket_create, setsockopt,
                dns::getaddrinfo_numeric_datagram, sendto, close_mock>(
                    "8.8.8.8", "53", "invalid", Logger::global()
                );
        REQUIRE(!!maybe_sock);
    }
    REQUIRE(close_called);
}
#endif

static void pollfd_fail(socket_t, short, Callback<Error, short> cb,
                        double, Var<Reactor> reactor) {
    reactor->call_soon([=]() {
        cb(MockedError(), 0);
    });
}

TEST_CASE("dns::pollin() handles reactor->pollfd() failure") {
    Var<Reactor> reactor = Reactor::make();
    Var<socket_t> sock{new socket_t{2}};
    reactor->loop_with_initial_event([=]() {
        dns::pollin_impl<pollfd_fail>(sock, [=](Error err) {
            REQUIRE(err.code == MockedError().code);
            reactor->break_loop();
        }, Settings{}, reactor, Logger::global());
    });
}

static void pollfd_invalid(socket_t, short, Callback<Error, short> cb,
                        double, Var<Reactor> reactor) {
    reactor->call_soon([=]() {
        cb(NoError(), MK_POLLOUT);
    });
}

TEST_CASE("dns::pollin() handles unexpected reactor->pollfd() flags") {
    Var<Reactor> reactor = Reactor::make();
    Var<socket_t> sock{new socket_t{2}};
    reactor->loop_with_initial_event([=]() {
        dns::pollin_impl<pollfd_invalid>(sock, [=](Error err) {
            REQUIRE(err.code == dns::UnexpectedPollFlagsError().code);
            reactor->break_loop();
        }, Settings{}, reactor, Logger::global());
    });
}

static ssize_t recv_fail(socket_t, void *, size_t, int) {
    return -1;
}

TEST_CASE("dns::recv() handles recv() failure") {
    Var<socket_t> sock{new socket_t{2}};
    ErrorOr<std::string> maybe_str =
        dns::recv_impl<recv_fail>(sock, Logger::global());
    REQUIRE(!maybe_str);
    REQUIRE(maybe_str.as_error().code == dns::RecvError().code);
}

static ssize_t recv_short_read(socket_t, void *, size_t, int) {
    return 0;
}

TEST_CASE("dns::recv() handles recv() short read") {
    Var<socket_t> sock{new socket_t{2}};
    ErrorOr<std::string> maybe_str =
        dns::recv_impl<recv_short_read>(sock, Logger::global());
    REQUIRE(!maybe_str);
    REQUIRE(maybe_str.as_error().code == dns::UnexpectedShortReadError().code);
}

static ErrorOr<Var<socket_t>>
send_fail(std::string, std::string, std::string, Var<Logger>) {
    return MockedError();
}

TEST_CASE("dns::sendrecv() handles send() failure") {
    Var<Reactor> reactor = Reactor::make();
    reactor->loop_with_initial_event([=]() {
        mk::dns::sendrecv_impl<send_fail>(
            "8.8.8.8", "53", "invalid", [=](Error error, std::string s) {
                REQUIRE(error.code == MockedError().code);
                REQUIRE(s == "");
                reactor->break_loop();
            }, {}, reactor, Logger::global());
    });
}

static ErrorOr<Var<socket_t>>
send_okay(std::string, std::string, std::string, Var<Logger>) {
    return NoError();
}

static void pollin_fail(Var<socket_t>, Callback<Error> callback, Settings,
                        Var<Reactor>, Var<Logger>) {
    callback(MockedError());
}

TEST_CASE("dns::sendrecv() handles pollin() failure") {
    Var<Reactor> reactor = Reactor::make();
    reactor->loop_with_initial_event([=]() {
        mk::dns::sendrecv_impl<send_okay, pollin_fail>(
            "8.8.8.8", "53", "invalid", [=](Error error, std::string s) {
                REQUIRE(error.code == MockedError().code);
                REQUIRE(s == "");
                reactor->break_loop();
            }, {}, reactor, Logger::global());
    });
}

static void pollin_okay(Var<socket_t>, Callback<Error> callback, Settings,
                        Var<Reactor>, Var<Logger>) {
    callback(NoError());
}

static ErrorOr<std::string> recv_failure(Var<socket_t>, Var<Logger>) {
    return MockedError();
}

TEST_CASE("dns::sendrecv() handles recv() failure") {
    Var<Reactor> reactor = Reactor::make();
    reactor->loop_with_initial_event([=]() {
        mk::dns::sendrecv_impl<send_okay, pollin_okay, recv_failure>(
            "8.8.8.8", "53", "invalid", [=](Error error, std::string s) {
                REQUIRE(error.code == MockedError().code);
                REQUIRE(s == "");
                reactor->break_loop();
            }, {}, reactor, Logger::global());
    });
}
