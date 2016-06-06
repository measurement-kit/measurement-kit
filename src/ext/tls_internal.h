/*
 * This is a mock of tls_internal.h filling out all the functions that are
 * called by tls_verify with mocks.
 */
#ifndef SRC_EXT_TLS_INTERNAL_H
#define SRC_EXT_TLS_INTERNAL_H

#include <openssl/x509.h>
#include <netinet/in.h>

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

int tls_check_name(struct tls *ctx, X509 *cert, const char *name);

#ifdef __cplusplus
}
#endif

/* Empty macro definitions to mock out the tls_set_error* functions */
#define tls_set_errorx(ctx, errmsg, name)
#define tls_set_error(ctx, errmsg, name)

#endif /* SRC_EXT_TLS_INTERNAL_H */
