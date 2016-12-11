// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NET_SSL_CONTEXT_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_NET_SSL_CONTEXT_IMPL_HPP

#include "../net/ssl_context.hpp"
#include <cassert>
#include <openssl/err.h>

namespace mk {
namespace net {

template <MK_MOCK(SSL_new)>
ErrorOr<SSL *> make_ssl(SSL_CTX *ctx, std::string hostname) {
    assert(ctx != nullptr);
    SSL *ssl = SSL_new(ctx);
    if (ssl == nullptr) {
        warn("ssl: failed to call SSL_new");
        return SslNewError();
    }
    SSL_set_tlsext_host_name(ssl, hostname.c_str());
    return ssl;
}

static void initialize_ssl() {
    static bool ssl_initialized = false;
    if (!ssl_initialized) {
        SSL_library_init();
        ERR_load_crypto_strings();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        ssl_initialized = true;
    }
}

template <MK_MOCK(SSL_CTX_new), MK_MOCK(SSL_CTX_load_verify_locations)>
ErrorOr<SSL_CTX *> make_ssl_ctx(std::string path) {

    debug("ssl: creating ssl context with bundle %s", path.c_str());
    initialize_ssl();

    if (path == "") {
        return MissingCaBundlePathError();
    }

    SSL_CTX *ctx = SSL_CTX_new(TLSv1_client_method());
    if (ctx == nullptr) {
        debug("ssl: failed to create SSL_CTX");
        return SslCtxNewError();
    }

    if (SSL_CTX_load_verify_locations(ctx, path.c_str(), nullptr) != 1) {
        debug("ssl: failed to load verify location");
        // XXX: not freeing to see what happens with Valgrind
        return SslCtxLoadVerifyLocationsError();
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
    return ctx;
}

} // namespace libevent
} // namespace mk
#endif
