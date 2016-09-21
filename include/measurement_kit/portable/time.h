/*
 * Public domain.
 */
#ifndef MEASUREMENT_KIT_PORTABLE_TIME_H
#define MEASUREMENT_KIT_PORTABLE_TIME_H

#include <time.h>

#ifdef _WIN32
#ifdef __cplusplus
extern "C" {
#endif

struct tm *mkp_gmtime_r(const time_t *clock, struct tm *result);

#ifdef __cplusplus
}
#endif

#else
#define mkp_gmtime_r gmtime_r
#endif // _WIN32

#endif
