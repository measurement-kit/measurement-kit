// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>
#include <exception>
#include "src/net/ssl-wrappers.hpp"

namespace mk {
namespace net {

SSL_CTX *WrapperSsl::get_client_context() {
    static WrapperSsl singleton;
    return singleton.ctx;
}

SSL *WrapperSsl::get_client_ssl() {
    SSL *ssl = SSL_new(get_client_context());
    if (ssl == nullptr) {
        throw std::exception();
    }
    return ssl;
}

WrapperSsl::WrapperSsl() {
    SSL_library_init();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    // FIXME: the code in here is really simple because we only aim to
    // smoke test the OpenSSL functionality. What is missing here is at
    // the minimum code to verify the peer certificate. Also we should
    // not allow SSLv2 (and probably v3) connections, and we should also
    // probably set SNI for the other end to respond correctly.
    ctx = SSL_CTX_new(SSLv23_client_method());
    if (ctx == nullptr) {
        throw std::exception();
    }
}

WrapperSsl::~WrapperSsl() { SSL_CTX_free(ctx); }

} // namespace net
} // namespace mk
