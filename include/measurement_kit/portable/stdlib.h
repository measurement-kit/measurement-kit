/*
 * Public domain.
 */
#ifndef MEASUREMENT_KIT_PORTABLE_STDLIB_H
#define MEASUREMENT_KIT_PORTABLE_STDLIB_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

long long strtonum(const char *, long long, long long, const char **);

#ifdef MKP_STDLIB_VISIBLE

long long mkp_strtonum(const char *numstr, long long minval, long long maxval,
                       const char **errstrp);

#endif
#ifdef __cplusplus
}
#endif
#endif
