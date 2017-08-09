// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_NET_SSL_CONTEXT_HPP
#define PRIVATE_NET_SSL_CONTEXT_HPP

#include <measurement_kit/net.hpp>
#include <openssl/ssl.h>

namespace mk {
namespace net {

class SslContext : public NonCopyable, public NonMovable {
  public:
    ErrorOr<SSL *> get_client_ssl(std::string hostname);

    ~SslContext();

    static ErrorOr<Var<SslContext>> make(std::string ca_bundle_path);

  private:
    SslContext() {}
    Error init(std::string ca_bundle_path);

    std::string ca_bundle_path; // Used to decide whether to recreate singleton
    ssl_ctx_st *ctx = nullptr;
};

} // namespace net
} // namespace mk
#endif
