// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>
#include <stdexcept>
#include <iostream>
#include <measurement_kit/net.hpp>
#include "../libevent/ssl_context.hpp"

namespace mk {
namespace libevent {

using namespace mk::net;

ErrorOr<SSL *> SslContext::get_client_ssl(std::string hostname) {
    SSL *ssl = SSL_new(ctx);
    if (ssl == nullptr) {
        warn("ssl: failed to call SSL_new");
        return SslNewError();
    }
    SSL_set_tlsext_host_name(ssl, hostname.c_str());
    return ssl;
}

Error SslContext::init(std::string ca_bundle_path) {

    static bool ssl_initialized = false;
    if (!ssl_initialized) {
        SSL_library_init();
        ERR_load_crypto_strings();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        ssl_initialized = true;
    }

    debug("ssl: creating ssl context with bundle %s", ca_bundle_path.c_str());

    if (ca_bundle_path == "") {
        return MissingCaBundlePathError();
    }

    ctx = SSL_CTX_new(TLSv1_client_method());
    if (ctx == nullptr) {
        debug("ssl: failed to create SSL_CTX");
        return SslCtxNewError();
    }

    if (SSL_CTX_load_verify_locations(ctx, ca_bundle_path.c_str(), NULL) != 1) {
        debug("ssl: failed to load verify location");
        // Note: no need to free `ctx` because it is owned by the `this` and
        // therefore will be destroyed when object exits the scope
        return SslCtxLoadVerifyLocationsError();
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    this->ca_bundle_path = ca_bundle_path;
    return NoError();
}

/*static */ ErrorOr<Var<SslContext>> SslContext::make(std::string path) {
    static Var<SslContext> singleton;
    // We basically cache the last created context and we reuse it if the path
    // has not been changed, otherwise we create a new context
    if (!singleton or singleton->ca_bundle_path != path) {
        singleton.reset(new SslContext);
        Error err = singleton->init(path);
        if (err) {
            singleton.reset(); // Basically: "this is Sparta!"
            return err;
        }
        /* FALLTHROUGH */
    }
    return singleton;
}

SslContext::~SslContext() {
    SSL_CTX_free(ctx);
}

} // namespace libevent
} // namespace mk

