// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../src/libmeasurement_kit/dns/getaddrinfo_impl.hpp"

using namespace mk;

static int getaddrinfo_fail(const char *, const char *, const addrinfo *,
                            addrinfo **) {
    return 1; /* Any nonzero value should mean failure */
}

TEST_CASE("dns::getaddrinfo() handles ::getaddrinfo errors") {
    addrinfo hints = {};
    ErrorOr<Var<addrinfo>> maybe_ainfo =
        dns::getaddrinfo_impl<getaddrinfo_fail>(
            "www.google.com", "80", &hints, Logger::global()
        );
    REQUIRE(!maybe_ainfo);
    REQUIRE(maybe_ainfo.as_error().code != NoError().code);
}

#ifdef ENABLE_INTEGRATION_TESTS
static bool freeaddrinfo_called = false;
static void freeaddrinfo_mock(addrinfo *ainfo) {
    freeaddrinfo_called = true;
    freeaddrinfo(ainfo);
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
TEST_CASE("Make sure freeaddrinfo() is called") {
    {
        addrinfo hints = {};
        ErrorOr<Var<addrinfo>> maybe_ainfo =
            dns::getaddrinfo_impl<getaddrinfo, freeaddrinfo_mock>(
                "www.google.com", "80", &hints, Logger::global()
            );
        REQUIRE(maybe_ainfo);
        REQUIRE(maybe_ainfo.as_error() == NoError());
    }
    REQUIRE(freeaddrinfo_called);
}
#endif
