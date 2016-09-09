/*
 * Public domain, 2013 Simone Basso.
 */

#ifdef __cplusplus
extern "C" {
#endif

long long mk_strtonum(const char *, long long, long long, const char **);

#ifdef __cplusplus
}
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STRTONUM
#define mk_strtonum strtonum
#endif
