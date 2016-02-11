// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_WRAPPERS_OPENSSL_HPP
#define SRC_WRAPPERS_OPENSSL_HPP

// Forward declarations
struct ssl_st;
struct ssl_ctx_st;

namespace mk {
namespace net {

class WrapperSsl {
  public:
    static ssl_ctx_st *get_client_context();

    static ssl_st *get_client_ssl();

  private:
    WrapperSsl();
    ~WrapperSsl();

    ssl_ctx_st *ctx = nullptr;
};

} // namespace net
} // namespace mk
#endif
