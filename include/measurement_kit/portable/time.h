/*
 * Public domain
 */
#ifndef MEASUREMENT_KIT_PORTABLE_TIME_H
#define MEASUREMENT_KIT_PORTABLE_TIME_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
/*
 * Provide prototype for `gmtime_r` which is missing on Windows.
 */
struct tm *gmtime_r(const time_t *clock, struct tm *result);
#endif

#ifdef MKP_TIME_VISIBLE

struct tm *mkp_gmtime_r(const time_t *clock, struct tm *result);

#endif
#ifdef __cplusplus
}
#endif
#endif
