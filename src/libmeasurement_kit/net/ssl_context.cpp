// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../net/ssl_context_impl.hpp"

namespace mk {
namespace net {

ErrorOr<SSL *> SslContext::get_client_ssl(std::string hostname) {
    return make_ssl(ctx, hostname);
}

/*static */ ErrorOr<Var<SslContext>> SslContext::make(std::string path) {
    static Var<SslContext> singleton;
    // We basically cache the last created context and we reuse it if the path
    // has not been changed, otherwise we create a new context
    if (!singleton or singleton->ca_bundle_path != path) {
        singleton.reset(new SslContext);
        ErrorOr<SSL_CTX *> maybe_ctx = make_ssl_ctx(path);
        if (!maybe_ctx) {
            singleton.reset();
            return maybe_ctx.as_error();
        }
        singleton->ctx = *maybe_ctx;
        singleton->ca_bundle_path = path;
        /* FALLTHROUGH */
    }
    return singleton;
}

SslContext::~SslContext() {
    SSL_CTX_free(ctx);
}

} // namespace libevent
} // namespace mk
