/*
 * Public domain
 */
#ifndef MEASUREMENT_KIT_PORTABLE_STRINGS_H
#define MEASUREMENT_KIT_PORTABLE_STRINGS_H

#ifndef _WIN32
#include <strings.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int strcasecmp(const char *s1, const char *s2);

#ifdef __cplusplus
}
#endif
#endif
