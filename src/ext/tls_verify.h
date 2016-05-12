#ifndef HEADER_TLS_VERIFY_H
#define HEADER_TLS_VERIFY_H

#include <openssl/x509.h>

struct tls {
    char *errmsg;
    int errnum;
};

#ifdef __cplusplus
extern "C" {
#endif

int tls_check_name(struct tls *ctx, X509 *cert, const char *name);

#ifdef __cplusplus
}
#endif

#endif /* HEADER_TLS_VERIFY_H */
