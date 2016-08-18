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
#include "src/net/ssl-context.hpp"
#include "config.h"

namespace mk {
namespace net {

SSL *SslContext::get_client_ssl(std::string hostname) {
    SSL *ssl = SSL_new(ctx);
    if (ssl == nullptr) {
        warn("ssl: failed to call SSL_new");
        throw std::runtime_error("SSL_new() failed");
    }

    SSL_set_tlsext_host_name(ssl, hostname.c_str());
    return ssl;
}

void SslContext::init(std::string ca_bundle_path) {
    static bool ssl_initialized = false;

    if (!ssl_initialized) {
        SSL_library_init();
        ERR_load_crypto_strings();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        ssl_initialized = true;
    }

    debug("ssl: creating ssl context with bundle %s", ca_bundle_path.c_str());

    ctx = SSL_CTX_new(TLSv1_client_method());
    if (ctx == nullptr) {
        debug("ssl: failed to create SSL_CTX");
        throw std::runtime_error("Failed to create SSL_CTX");
    }

    if (SSL_CTX_load_verify_locations(ctx, ca_bundle_path.c_str(), NULL) != 1) {
        debug("ssl: failed to load verify location");
        throw std::runtime_error("Failed to load verify location");
    };

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
}

Var<SslContext> SslContext::global() {
    static Var<SslContext> singleton(new SslContext);
    return singleton;
}

SslContext::SslContext() {
    std::string ca_bundle_path;
    Var<Settings> global_settings = Settings::global();
    if (global_settings->find("net/ca_bundle_path") != global_settings->end()) {
        ca_bundle_path = (*global_settings)["net/ca_bundle_path"];
    } else {
    #ifdef MK_CA_BUNDLE
        ca_bundle_path = MK_CA_BUNDLE;
    #else 
        warn("ssl: failed to find ca_bundle");
        throw MissingCaBundlePathError();
    #endif
    }

    init(ca_bundle_path);
}

SslContext::SslContext(std::string ca_bundle_path) {
    init(ca_bundle_path);
}

SslContext::~SslContext() {
    SSL_CTX_free(ctx);
}

} // namespace net
} // namespace mk

