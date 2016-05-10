// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>
#include <exception>
#include <iostream>
#include <measurement_kit/net.hpp>
#include "src/net/ssl-context.hpp"

namespace mk {
namespace net {

SSL *SslContext::get_client_ssl(std::string hostname) {
    SSL *ssl = SSL_new(ctx);
    if (ssl == nullptr) {
        debug("ssl: failed to call SSL_new");
        throw std::exception();
    }

    SSL_set_tlsext_host_name(ssl, hostname.c_str());
    return ssl;
}

void SslContext::init(std::string ca_bundle_path) {
    SSL_library_init();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    debug("ssl: creating ssl context with bundle %s", ca_bundle_path.c_str());

    ctx = SSL_CTX_new(TLSv1_client_method());
    if (ctx == nullptr) {
        debug("ssl: failed to create SSL_CTX");
        throw std::exception();
    }

    if (SSL_CTX_load_verify_locations(ctx, ca_bundle_path.c_str(), NULL) != 1) {
        debug("ssl: failed to load verify location");
        throw std::exception();
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
        // XXX should we add other system default locations in here?
        warn("ssl: failed to find ca_bundle");
        throw MissingCaBundlePathError();
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

