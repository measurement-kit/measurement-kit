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

#ifdef __cplusplus
}
#endif
#endif
