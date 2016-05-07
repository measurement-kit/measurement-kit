/*
 * This is a mock of tls_internal.h filling out all the functions that are
 * called by tls_verify with mocks.
 */
#ifndef HEADER_TLS_INTERNAL_MOCK_H
#define HEADER_TLS_INTERNAL_MOCK_H

#include "tls_verify.h"

/* Empty macro definitions to mock out the tls_set_error* functions
 */

#define tls_set_errorx(ctx, errmsg, name)
#define tls_set_error(ctx, errmsg, name)

union tls_addr {
    struct in_addr ip4;
    struct in6_addr ip6;
};

#endif /* HEADER_TLS_INTERNAL_MOCK_H */
