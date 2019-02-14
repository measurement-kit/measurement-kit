// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include "src/libmeasurement_kit/net/libssl.hpp"
#include <future>

using namespace mk::net::libssl;
using namespace mk::net;
using namespace mk;

static const char *default_cert = "./test/fixtures/saved_ca_bundle.pem";

static SSL *ssl_new_fail(SSL_CTX *) { return nullptr; }

TEST_CASE("Context::get_client_ssl() works") {
    SECTION("when SSL_new() fails") {
        auto context = Context::make(default_cert, Logger::make());
        auto maybe_ssl = (*context)->get_client_ssl<ssl_new_fail>(
              "www.google.com", Logger::make());
        REQUIRE(!maybe_ssl);
        REQUIRE(maybe_ssl.as_error() == SslNewError());
    }
}

static SSL_CTX *ssl_ctx_new_fail(const SSL_METHOD *) { return nullptr; }

static int ssl_ctx_load_verify_locations_fail(SSL_CTX *, const char *,
                                              const char *) {
    return 0;
}

TEST_CASE("Context::make() works") {
    SECTION("when the ca_bundle_path is empty") {
        auto maybe_ctx = Context::make("", Logger::make());
        REQUIRE(!maybe_ctx);
        REQUIRE(maybe_ctx.as_error() == MissingCaBundlePathError());
    }

    SECTION("when SSL_CTX_new() fails") {
        auto maybe_ctx = Context::make<ssl_ctx_new_fail>("/nonexistent", Logger::make());
        REQUIRE(!maybe_ctx);
        REQUIRE(maybe_ctx.as_error() == SslCtxNewError());
    }

    SECTION("when SSL_CTX_load_verify_locations() fails") {
        auto maybe_ctx =
              Context::make<SSL_CTX_new, ssl_ctx_load_verify_locations_fail>(
                    default_cert, Logger::make());
        REQUIRE(!maybe_ctx);
        REQUIRE(maybe_ctx.as_error() == SslCtxLoadVerifyLocationsError());
    }
}

static ErrorOr<SharedPtr<Context>> context_make_fail(std::string, SharedPtr<Logger>) {
    return {MockedError(), {}};
}

TEST_CASE("Cache works as expected") {
    SECTION("different threads get different SSL_CTX") {
        auto make = []() {
            return Cache<>::thread_local_instance()->get_client_ssl(
                  default_cert, "www.google.com", Logger::make());
        };
        auto first = std::async(std::launch::async, make).get();
        auto second = std::async(std::launch::async, make).get();
        REQUIRE(*first != *second);
        SSL_free(*first);
        SSL_free(*second);
    }

    SECTION("cache is evicted when too many SSL_CTX are created") {
        auto cache = Cache<1>{};
        REQUIRE(cache.size() == 0);
        auto ssl = cache.get_client_ssl(default_cert, "www.google.com",
                                        Logger::make());
        REQUIRE(*ssl != nullptr);
        REQUIRE(cache.size() == 1);
        SSL_free(*ssl);
        ssl = cache.get_client_ssl("./test/fixtures/saved_ca_bundle.pem",
                                   "www.google.com", Logger::make());
        REQUIRE(*ssl != nullptr);
        // Note: we expect this to be equal to one, meaning that the first
        // created SSL_CTX has been evicted, given cache size.
        REQUIRE(cache.size() == 1);
        SSL_free(*ssl);
    }

    SECTION("cache works in the common case") {
        auto cache = Cache<>{};
        REQUIRE(cache.size() == 0);
        auto ssl = cache.get_client_ssl(default_cert, "www.google.com",
                                        Logger::make());
        REQUIRE(*ssl != nullptr);
        REQUIRE(cache.size() == 1);
        SSL_free(*ssl);
        ssl = cache.get_client_ssl("./test/fixtures/basic_ca.pem",
                                   "www.google.com", Logger::make());
        REQUIRE(*ssl != nullptr);
        REQUIRE(cache.size() == 2);
        SSL_free(*ssl);
    }

    SECTION("cache uses same SSL_CTX for equal certificate path") {
        auto cache = Cache<>{};
        REQUIRE(cache.size() == 0);
        auto first = cache.get_client_ssl(default_cert, "www.google.com",
                                          Logger::make());
        REQUIRE(!!first);
        auto second = cache.get_client_ssl(default_cert, "www.kernel.org",
                                           Logger::make());
        REQUIRE(!!second);
        REQUIRE(SSL_get_SSL_CTX(*first) == SSL_get_SSL_CTX(*second));
        SSL_free(*first);
        SSL_free(*second);
    }

    SECTION("cache behaves when Context::make fails") {
        auto cache = Cache<>{};
        auto r = cache.get_client_ssl<context_make_fail>(
              default_cert, "www.google.com", Logger::make());
        REQUIRE(!r);
        REQUIRE(r.as_error() == MockedError());
    }
}

static long ssl_get_verify_result_fail(const SSL *) {
    return X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT;
}

static long ssl_get_verify_result_success(const SSL *) {
    return X509_V_OK;
}

static X509 *ssl_get_peer_certificate_fail(const SSL *) {
    return nullptr;
}

static X509 *ssl_get_peer_certificate_success(const SSL *) {
    return X509_new();
}

static int tls_check_name_fail(struct tls *, X509 *, const char *, int *) {
    return -1;
}

static int tls_check_name_nomatch(struct tls *, X509 *, const char *, int *x) {
    *x = 0;
    return 0;
}

TEST_CASE("verify_peer works as expected") {
    Cache<> c;

    SECTION("when SSL_get_verify_result fails") {
        auto ssl = *c.get_client_ssl(default_cert, "x.org", Logger::make());
        REQUIRE(verify_peer<ssl_get_verify_result_fail>(
                      "", ssl, Logger::make()) != NoError());
        SSL_free(ssl);
    }

    SECTION("when SSL_get_peer_certificate fails") {
        auto ssl = *c.get_client_ssl(default_cert, "x.org", Logger::make());
        REQUIRE((verify_peer<ssl_get_verify_result_success,
                             ssl_get_peer_certificate_fail>(
                      "", ssl, Logger::make())) != NoError());
        SSL_free(ssl);
    }

    SECTION("when tls_check_name fails") {
        auto ssl = *c.get_client_ssl(default_cert, "x.org", Logger::make());
        REQUIRE((verify_peer<ssl_get_verify_result_success,
                             ssl_get_peer_certificate_success,
                             tls_check_name_fail>("", ssl, Logger::make())) !=
                NoError());
        SSL_free(ssl);
    }

    SECTION("when tls_check_name does not match") {
        auto ssl = *c.get_client_ssl(default_cert, "x.org", Logger::make());
        REQUIRE((verify_peer<ssl_get_verify_result_success,
                             ssl_get_peer_certificate_success,
                             tls_check_name_nomatch>(
                      "", ssl, Logger::make())) != NoError());
        SSL_free(ssl);
    }
}
