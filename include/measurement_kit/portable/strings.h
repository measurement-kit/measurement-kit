/*
 * Public domain.
 */
#ifndef MEASUREMENT_KIT_PORTABLE_STRINGS_H
#define MEASUREMENT_KIT_PORTABLE_STRINGS_H

#ifndef _WIN32
#include <strings.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int mkp_strcasecmp(const char *s1, const char *s2);

int mkp_strncasecmp(const char *s1, const char *s2, size_t n);

#ifdef __cplusplus
}
#endif

#endif
