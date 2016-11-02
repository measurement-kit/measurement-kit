// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_LIBEVENT_SSL_CONTEXT_HPP
#define SRC_LIBMEASUREMENT_KIT_LIBEVENT_SSL_CONTEXT_HPP

#include <measurement_kit/common.hpp>

#include <openssl/ssl.h>

// Forward declarations
struct ssl_st;
struct ssl_ctx_st;

namespace mk {
namespace libevent {

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

} // namespace libevent
} // namespace mk
#endif
