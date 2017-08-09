// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/net/ssl_context_impl.hpp"

using namespace mk;

static SSL *ssl_new_fail(SSL_CTX *) { return nullptr; }

TEST_CASE("make_ssl() works") {

    net::initialize_ssl(); /* Required to avoid failure when calling API */

    SECTION("when SSL_new() fails") {
        SSL_CTX *ctx = SSL_CTX_new(SSLv23_method());
        REQUIRE(ctx != nullptr);
        ErrorOr<SSL *> maybe_ssl =
            net::make_ssl<ssl_new_fail>(ctx, "www.google.com");
        REQUIRE(!maybe_ssl);
        REQUIRE(maybe_ssl.as_error().code == net::SslNewError().code);
        SSL_CTX_free(ctx);
    }
}

static SSL_CTX *ssl_ctx_new_fail(const SSL_METHOD *) { return nullptr; }

static int ssl_ctx_load_verify_locations_fail(SSL_CTX *, const char *,
                                              const char *) {
    return 0;
}

#if (defined LIBRESSL_VERSION_NUMBER && LIBRESSL_VERSION_NUMBER >= 0x2010400fL)
static int ssl_ctx_load_verify_mem_fail(SSL_CTX *, void *, int) { return 0; }
#endif

TEST_CASE("make_ssl_ctx() works") {
    SECTION("when the ca_bundle_path is empty") {
#if (defined LIBRESSL_VERSION_NUMBER && LIBRESSL_VERSION_NUMBER >= 0x2010400fL)
        ErrorOr<SSL_CTX *> maybe_ctx = net::make_ssl_ctx("");
        REQUIRE(!!maybe_ctx);
        REQUIRE(*maybe_ctx != nullptr);
        SSL_CTX_free(*maybe_ctx);
#else
        ErrorOr<SSL_CTX *> maybe_ctx = net::make_ssl_ctx("");
        REQUIRE(!maybe_ctx);
        REQUIRE(maybe_ctx.as_error().code ==
                net::MissingCaBundlePathError().code);
#endif
    }

    SECTION("when SSL_CTX_new() fails") {
        ErrorOr<SSL_CTX *> maybe_ctx =
            net::make_ssl_ctx<ssl_ctx_new_fail>("/nonexistent");
        REQUIRE(!maybe_ctx);
        REQUIRE(maybe_ctx.as_error().code == net::SslCtxNewError().code);
    }

    SECTION("when SSL_CTX_load_verify_locations() fails") {
        ErrorOr<SSL_CTX *> maybe_ctx =
            net::make_ssl_ctx<SSL_CTX_new, ssl_ctx_load_verify_locations_fail>(
                "./text/fixtures/certs.pem");
        REQUIRE(!maybe_ctx);
        REQUIRE(maybe_ctx.as_error().code ==
                net::SslCtxLoadVerifyLocationsError().code);
    }

#if (defined LIBRESSL_VERSION_NUMBER && LIBRESSL_VERSION_NUMBER >= 0x2010400fL)
    SECTION("when SSL_CTX_load_verify_mem() fails") {
        ErrorOr<SSL_CTX *> maybe_ctx =
            net::make_ssl_ctx<SSL_CTX_new, SSL_CTX_load_verify_locations,
                              ssl_ctx_load_verify_mem_fail>("");
        REQUIRE(!maybe_ctx);
        REQUIRE(maybe_ctx.as_error().code ==
                net::SslCtxLoadVerifyMemError().code);
    }
#endif
}

TEST_CASE("SslContext::make() works as expected") {
    SECTION("when the path is nonexistent") {
        ErrorOr<Var<net::SslContext>> maybe_ctx =
            net::SslContext::make("/nonexistent");
        REQUIRE(!maybe_ctx);
        REQUIRE(maybe_ctx.as_error().code ==
                net::SslCtxLoadVerifyLocationsError().code);
    }

    SECTION("when the path exists and we cache it") {
        ErrorOr<Var<net::SslContext>> maybe_ctx_1 =
            net::SslContext::make("./test/fixtures/certs.pem");
        REQUIRE(!!maybe_ctx_1);
        ErrorOr<Var<net::SslContext>> maybe_ctx_2 =
            net::SslContext::make("./test/fixtures/certs.pem");
        REQUIRE(!!maybe_ctx_2);
        REQUIRE(maybe_ctx_1->get() == maybe_ctx_2->get());
    }
}
