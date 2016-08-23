// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_SSL_CONTEXT_HPP
#define SRC_NET_SSL_CONTEXT_HPP

#include <openssl/ssl.h>
#include <measurement_kit/common.hpp>

// Forward declarations
struct ssl_st;
struct ssl_ctx_st;

namespace mk {
namespace net {

class SslContext : public NonCopyable, public NonMovable {
  public:
    SSL *get_client_ssl(std::string hostname);

    ~SslContext();

    SslContext(std::string ca_bundle_path);

    SslContext();

    static Var<SslContext> global();

  private:
    void init(std::string ca_bundle_path);

    ssl_ctx_st *ctx = nullptr;
};

} // namespace net
} // namespace mk
#endif
