// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NET_LIBSSL_HPP
#define SRC_LIBMEASUREMENT_KIT_NET_LIBSSL_HPP

// ## libssl
/*-
 _ _ _             _
| (_) |__  ___ ___| |
| | | '_ \/ __/ __| |
| | | |_) \__ \__ \ |
|_|_|_.__/|___/___/_|
*/
/// \file src/libmeasurement_kit/net/libssl.hpp
/// \brief Code related to libssl (openssl or libressl).

#include "src/libmeasurement_kit/common/locked.hpp"
#include "src/libmeasurement_kit/common/mock.hpp"
#include "src/libmeasurement_kit/ext/tls_internal.h"
#include "src/libmeasurement_kit/net/builtin_ca_bundle.hpp"
#include <cassert>
#include <map>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/non_copyable.hpp>
#include <measurement_kit/common/non_movable.hpp>
#include "src/libmeasurement_kit/net/error.hpp"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string>

namespace mk {
namespace net {
namespace libssl {

/// Initialize the SSL library
static inline void libssl_init_once(SharedPtr<Logger> logger) {
    locked_global([logger]() {
        static bool initialized = false;
        if (!initialized) {
            logger->debug2("initializing libssl once");
            SSL_library_init();
            ERR_load_crypto_strings();
            SSL_load_error_strings();
            OpenSSL_add_all_algorithms();
            initialized = true;
        }
    });
}

/*!
    \brief Wrapper for SSL context (`SSL_CTX *`).

    This class wraps `SSL_CTX *` providing RAII destructor and allowing to
    create a `SSL *` from the managed `SSL_CTX *` object.
*/
class Context : public NonCopyable, public NonMovable {
  public:
    /*
        Implementation notes
        --------------------

        1.  libssl uses reference counting[1], meaning that the `SSL_CTX *`
        will be alive as long as there's a `SSL *` using it.

        2. this class does not take any measure to prevent access from multiple
        threads even though it is not generally wise to assume that OpenSSL
        will do what you expect on a MT environment [2].

        The policy for managing `Context`s in a MT environment should instead
        be enforced by the `Cache` implementation (below).

        3. since it manages a C pointer, this class MUST be non-copyable
        and non-movable to prevent multiple-free bugs.

        Links
        -----

        .. [1] https://wiki.openssl.org/index.php/Manual:SSL_CTX_free(3)
        .. [2] https://github.com/openssl/openssl/issues/2165
    */

    /*!
        \brief Creates a `SSL *` from the wrapped `SSL_CTX *`.

        \param hostname Hostname for which to create the `SSL *`. This would be
        the hostname for which you want to use SNI.

        \param logger The logger to use to emit log messages.

        \return An error on failure (the returned object will evaluate to false
        in a boolean context) and a `SSL *` on success.
    */
    template <MK_MOCK(SSL_new)>
    ErrorOr<SSL *> get_client_ssl(
            std::string hostname, SharedPtr<Logger> logger) {
        assert(ctx_ != nullptr);
        SSL *ssl = SSL_new(ctx_);
        if (ssl == nullptr) {
            logger->warn("ssl: SSL_new failed");
            return {SslNewError(), {}};
        }
        SSL_set_tlsext_host_name(ssl, hostname.c_str());
        return {NoError(), ssl};
    }

    /// Destructor.
    ~Context() {
        assert(ctx_ != nullptr);
        SSL_CTX_free(ctx_);
    }

    /*!
        \brief Factory that creates a shared `SSL_CTX *` wrapper.

        \param ca_bundle_path Path to the CA bundle to be used to verify
        the servers we're connecting to. If it is empty and we're linking
        with libressl, we will use the builtin CA bundle that is shipped
        along with measurement-kit. Otherwise, we will fail.

        \param logger Logger for printing log messages.

        \return An error on failure (the returned object will evaluate to
        false in a boolean context) or a `SharedPtr` shared pointer wrapping a
        Context instance on success.
    */
    template <MK_MOCK(SSL_CTX_new), MK_MOCK(SSL_CTX_load_verify_locations)
#if (defined LIBRESSL_VERSION_NUMBER && LIBRESSL_VERSION_NUMBER >= 0x2010400fL)
                                            ,
            MK_MOCK(SSL_CTX_load_verify_mem)
#endif
            >
    static ErrorOr<SharedPtr<Context>> make(
            std::string ca_bundle_path, SharedPtr<Logger> logger) {
        // Implementation note: we need to initialize libssl early otherwise
        // most of the functions of the library will not work properly.
        libssl_init_once(logger);
        logger->debug("ssl: creating SSL_CTX with bundle: '%s'",
                ca_bundle_path.c_str());
        SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
        if (ctx == nullptr) {
            logger->warn("ssl: failed to create SSL_CTX");
            return {SslCtxNewError(), {}};
        }
        /*
         * Implementation note:
         *
         * We use SSLv23_client_method() because it is the most general method
         * that applications SHOULD use according to the manual[1] and we
         * disable by default deprecated protocols. We will enable them
         * in the OONI's web_connectivity test so to measure more websites.
         *
         * [1] https://wiki.openssl.org/index.php/Manual:SSL_CTX_new(3)
         */
        SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
        if (ca_bundle_path != "") {
            if (!SSL_CTX_load_verify_locations(
                        ctx, ca_bundle_path.c_str(), nullptr)) {
                logger->warn("ssl: failed to load verify location");
                SSL_CTX_free(ctx);
                return {SslCtxLoadVerifyLocationsError(), {}};
            }
        } else {
#if (defined LIBRESSL_VERSION_NUMBER &&                                        \
        LIBRESSL_VERSION_NUMBER >= 0x2010400fL && !defined _MSC_VER)
            // Note: we disable the CA bundle on Windows where the compiler
            // fails with internal error when compiling the builtin vector that
            // contains the bytes of the CA file.
            std::vector<uint8_t> bundle = builtin_ca_bundle();
            logger->debug("ssl: using builtin libressl's ca bundle");
            if (!SSL_CTX_load_verify_mem(ctx, bundle.data(), bundle.size())) {
                logger->warn("ssl: failed to load default ca bundle");
                SSL_CTX_free(ctx);
                return {SslCtxLoadVerifyMemError(), {}};
            }
#else
            SSL_CTX_free(ctx);
            return {MissingCaBundlePathError(), {}};
#endif
        }
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
        SharedPtr<Context> context{new Context};
        context->ctx_ = ctx;
        return {NoError(), context};
    }

  private:
    Context() {}
    SSL_CTX *ctx_ = nullptr;
};

/*!
    \brief Thread-safe cache for constructing `SSL *` objects.

    \remark Unless you need low-level control, you SHOULD really use
    this API for creating an `SSL *` when connecting to an host.
*/
template <size_t max_cache_size = 64> class Cache {
  public:
    /*
        Implementation notes
        --------------------

        1. both on mobile and on desktop devices we should generally use just
        a single `SSL_CTX *`, either setup for using the bundled CA bundle (for
        mobile devices) or using a system-wide CA (desktop).

        However, for flexibility, we have allowed for many `SSL_CTX *`.

        To avoid the cache potentially becoming too large (perhaps also because
        of bugs) we have set a large hard limit on the maximum number of
        `SSL_CTX *` that we keep open concurrently.

        2. since assuming OpenSSL will correctly behave in a MT environment
        seem to be an over-assumption (see above for links), I decided to stay
        on the safe side an make the cache thread local.

        After #1297 is merged, this means that every "big" task (i.e. a test,
        orchestration, etc) will use its own thread local cache.

        Overall, this choice is a bit more defensive than needed, perhaps, but
        I do also see some advantages in not keeping `SSL_CTX *` - and hence
        all related caching - alive "forever". So I think it's okay.
    */

    /// Return thread local instance of the cache.
    ///
    /// @note We have experimentally found that iOS armv7 does not support well
    /// thread_local. In such case, we allocate a new cache every time rather
    /// than using a thread local cache. We will be able to drop this fix
    /// in the moment in which support for 32-bit iOS will be dropped.
    ///
    /// See <https://github.com/measurement-kit/measurement-kit/issues/1397>.
    static SharedPtr<Cache> thread_local_instance() {
#ifndef MK_NO_THREAD_LOCAL
        static thread_local
#endif
                SharedPtr<Cache>
                        instance{new Cache};
        return instance;
    }

    /// Inline wrapper for Context::make, for testability
    static inline ErrorOr<SharedPtr<Context>> mkctx(
            std::string ca_bundle_path, SharedPtr<Logger> logger) {
        return Context::make(ca_bundle_path, logger);
    }

    /*!
        \brief Factory to construct an `SSL *`.

        \param ca_bundle_path The CA bundle path to use. This should be set
        using the `net/ca_bundle_path` configuration variable.

        \param hostname Hostname for which you want to do SSL and which will
        be used in the SNI extension.

        \param logger Logger used to log messages.

        \return Error on failure (the returned object will be false when
        evaluated in a boolean context) or `SSL *` on success.

        \remark This factory gives you the ownership of the returned
        `SSL *` object and it is your responsibility to dispose of it
        when done. In practice, when using libevent, you will typically
        pass the `SSL *` to a bufferevent that will own it.

        \remark This operation is thread safe by definition since the
        cache is created using thread-local storage.
    */
    template <MK_MOCK(mkctx)>
    ErrorOr<SSL *> get_client_ssl(std::string ca_bundle_path,
            std::string hostname, SharedPtr<Logger> logger) {
        /*
           Implementation note: `SSL_CTX *` are reference counted and they will
           be alive as long as there is an `SSL *` using them. As such, we do
           not need to worry about `all_.clear()` causing `SSL_CTX_free` to be
           called on the `SSL_CTX *` we indirectly manage, because the usage
           count semantic will keep us safe from premature frees.

           See above for a specific link re: refcounting.
        */
        if (all_.size() >= max_cache_size) {
            logger->warn("ssl: hit hard limit of maximum cached SSL_CTX");
            all_.clear();
        }
        if (all_.count(ca_bundle_path) == 0) {
            ErrorOr<SharedPtr<Context>> maybe_context =
                    mkctx(ca_bundle_path, logger);
            if (!maybe_context) {
                return {maybe_context.as_error(), {}};
            }
            logger->debug2("ssl: track ctx for: '%s'", ca_bundle_path.c_str());
            all_[ca_bundle_path] = *maybe_context;
        }
        SharedPtr<Context> context = all_[ca_bundle_path];
        return context->get_client_ssl(hostname, logger);
    }

    /// Return number of cached SSL_CTX
    size_t size() const { return all_.size(); }

    /// Constructor
    Cache() {}

  private:
    std::map<std::string, SharedPtr<Context>> all_;
};

/*!
    \brief Verify SSL peer.

    \param hostname Expected SNI hostname.

    \param ssl Pointer to SSL struct.

    \param logger Logger used for logging.

    \return NoError() on success, an error on failure.
*/
template <MK_MOCK(SSL_get_verify_result), MK_MOCK(SSL_get_peer_certificate),
        MK_MOCK(tls_check_name)>
Error verify_peer(std::string hostname, SSL *ssl, SharedPtr<Logger> logger) {
    assert(ssl != nullptr);
    logger->debug("SSL version: %s", SSL_get_version(ssl));
    long verify_err = SSL_get_verify_result(ssl);
    if (verify_err != X509_V_OK) {
        logger->warn("ssl: got an invalid certificate");
        return SslInvalidCertificateError(
                X509_verify_cert_error_string(verify_err));
    }
    X509 *server_cert = SSL_get_peer_certificate(ssl);
    if (server_cert == nullptr) {
        logger->warn("ssl: got no certificate");
        return SslNoCertificateError();
    }
    auto match = 0;
    auto err = tls_check_name(nullptr, server_cert, hostname.c_str(), &match);
    X509_free(server_cert); // Make sure we don't leak memory
    if (err != 0) {
        logger->warn("ssl: got invalid hostname");
        return SslInvalidHostnameError();
    }
    if (!match) {
        logger->warn("ssl: name not present in server certificate");
        return SslMissingHostnameError();
    }
    return NoError();
}

/// Enable SSLv2 and SSLv3 for the selected connection
static inline void enable_v23(SSL *ssl) {
    SSL_clear_options(ssl, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
}

} // namespace libssl
} // namespace net
} // namespace mk
#endif
