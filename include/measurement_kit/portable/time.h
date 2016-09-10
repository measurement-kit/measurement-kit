/*
 * Public domain
 */
#ifndef MEASUREMENT_KIT_PORTABLE_TIME_H
#define MEASUREMENT_KIT_PORTABLE_TIME_H

#include <time.h>

#ifdef _WIN32

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Provide prototype for `gmtime_r` which is missing on Windows.
 */
struct tm *gmtime_r(const time_t *clock, struct tm *result);

#ifdef __cplusplus
}
#endif

#endif
#endif
