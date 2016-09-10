/*
 * Public domain
 */
#ifndef MEASUREMENT_KIT_PORTABLE_STRINGS_H
#define MEASUREMENT_KIT_PORTABLE_STRINGS_H

#ifdef _WIN32

#ifdef __cplusplus
extern "C" {
#endif

int strcasecmp(const char *s1, const char *s2);

#ifdef __cplusplus
}
#endif

#else

#include <strings.h>

#endif
#endif
