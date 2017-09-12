/*
 * Public domain.
 */
#ifndef MEASUREMENT_KIT_PORTABLE_STDLIB_H
#define MEASUREMENT_KIT_PORTABLE_STDLIB_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* strtonum() function is not defined under many C libraries but is defined
   e.g. by OpenBSD. Therefore we have our own wrapper that will call the sys
   strtonum() if available and otherwise use a replacement. */
long long mkp_strtonum(const char *numstr,
                       long long minval,
                       long long maxval,
                       const char **errstrp);

#ifdef __cplusplus
}
#endif
#endif
