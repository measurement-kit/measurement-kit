// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>
#include <exception>
#include <iostream>
#include "src/net/ssl-context.hpp"

namespace mk {
namespace net {

static const char *get_ca_file_location() {
    return "/tmp/cacert.pem";
}


SSL_CTX *SslContext::get_client_context() {
    static SslContext singleton;
    return singleton.ctx;
}

SSL *SslContext::get_client_ssl(std::string hostname) {
    SSL *ssl = SSL_new(get_client_context());
    if (ssl == nullptr) {
        throw std::exception();
    }

    // This is required to setup SNI (perhaps we should do this only when we
    // see it's a hostname?)
    SSL_set_tlsext_host_name(ssl, hostname.c_str());
    return ssl;
}

SslContext::SslContext() {
    // The return values of the following function calls do not matter.
    SSL_library_init();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    // XXX we may want to only load the algorithms we need to use.
    OpenSSL_add_all_algorithms();

    // FIXME: the code in here is really simple because we only aim to
    // smoke test the OpenSSL functionality. What is missing here is at
    // the minimum code to verify the peer certificate. Also we should
    // not allow SSLv2 (and probably v3) connections, and we should also
    // probably set SNI for the other end to respond correctly.
    ctx = SSL_CTX_new(TLSv1_client_method());
    if (ctx == nullptr) {
        throw std::exception();
    }
    if (SSL_CTX_load_verify_locations(ctx, get_ca_file_location(), NULL) == 0) {
         throw std::exception();
    };

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
}

SslContext::~SslContext() {
    EVP_cleanup();
    SSL_CTX_free(ctx);
}

} // namespace net
} // namespace mk

