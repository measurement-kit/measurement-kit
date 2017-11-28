/*
 * This is a mock of tls_internal.h filling out all the functions that are
 * called by tls_verify with mocks.
 */
#ifndef PRIVATE_EXT_TLS_INTERNAL_H
#define PRIVATE_EXT_TLS_INTERNAL_H

#include <measurement_kit/common/aaa_base.h>
#include <openssl/x509.h>

struct tls {
    char *errmsg;
    int errnum;
};

union tls_addr {
    struct in_addr ip4;
    struct in6_addr ip6;
};

#ifdef __cplusplus
extern "C" {
#endif

int tls_check_name(struct tls *ctx, X509 *cert, const char *name, int *match);

#ifdef __cplusplus
}
#endif

/*
 * Empty mocks for the tls_set_error* functions. One of them must be a
 * function such that we can ignore arguments. This is done to ensure
 * the compiler doesn't report warnings for `ctx` being unused.
 */

static inline void
tls_set_errorx(struct tls *ctx, const char *errmsg, ...)
               __attribute__((format(printf, 2, 3)));

static inline void
tls_set_errorx(struct tls *ctx, const char *errmsg, ...) {
    (void)ctx;
    (void)errmsg;
    /* do nothing */ ;
}
#define tls_set_error tls_set_errorx

#endif /* SRC_EXT_TLS_INTERNAL_H */
