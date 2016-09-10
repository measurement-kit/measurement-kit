/*
 * Public domain
 */
#ifndef MEASUREMENT_KIT_PORTABLE_SYS_TIME_H
#define MEASUREMENT_KIT_PORTABLE_SYS_TIME_H

#ifdef _WIN32
#include <measurement_kit/portable/_windows.h>
#else
#include <sys/time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int gettimeofday(struct timeval *tvp, void *tzp);

#ifdef __cplusplus
}
#endif
#endif
