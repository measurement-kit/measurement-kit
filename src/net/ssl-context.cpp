// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>
#include <exception>
#include <iostream>
#include <measurement_kit/common.hpp>
#include "src/net/ssl-context.hpp"

namespace mk {
namespace net {

Var<SslContext> SslContext::default_context() {
    static SslContext singleton;
    return Var<SslContext>(&singleton);
}

Var<SslContext> SslContext::make(std::string ca_bundle_path) {
    SslContext *ssl_context = new SslContext(ca_bundle_path);
    return Var<SslContext>(ssl_context);
}


SSL *SslContext::get_client_ssl(std::string hostname) {
    SSL *ssl = SSL_new(ctx);
    if (ssl == nullptr) {
        Logger::global()->debug("ssl: failed to call SSL_new");
        throw std::exception();
    }

    // This is required to setup SNI (perhaps we should do this only when we
    // see it's a hostname?)
    SSL_set_tlsext_host_name(ssl, hostname.c_str());
    return ssl;
}

void SslContext::init(std::string ca_bundle_path) {
    SSL_library_init();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    ctx = SSL_CTX_new(TLSv1_client_method());
    if (ctx == nullptr) {
        Logger::global()->debug("ssl: failed to create SSL_CTX");
        throw std::exception();
    }

    if (SSL_CTX_load_verify_locations(ctx, ca_bundle_path.c_str(), NULL) == 0) {
        Logger::global()->debug("ssl: failed to load verify location");
         throw std::exception();
    };

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
}

SslContext::SslContext() {
    std::string ca_bundle_path;
    Settings *global_settings = Settings::global();
    if (global_settings->find("ca_bundle_path") != global_settings->end()) {
        ca_bundle_path = (*global_settings)["ca_bundle_path"];
    } else {
        // XXX should we add other system default locations in here?
        Logger::global()->debug("ssl: failed to find ca_bundle");
        throw std::exception();
    }
    init(ca_bundle_path);
}

SslContext::SslContext(std::string ca_bundle_path) {
    init(ca_bundle_path);
}

SslContext::~SslContext() {
    EVP_cleanup();
    SSL_CTX_free(ctx);
}

} // namespace net
} // namespace mk

