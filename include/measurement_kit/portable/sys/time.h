/*
 * Public domain
 */
#ifndef MEASUREMENT_KIT_PORTABLE_SYS_TIME_H
#define MEASUREMENT_KIT_PORTABLE_SYS_TIME_H

#ifdef _WIN32

/*
 * 1. Pull `struct timeval`.
 */
#include <winsock2.h>

#ifdef __cplusplus
extern "C" {
#endif

int gettimeofday(struct timeval *tvp, void *tzp);

#ifdef __cplusplus
}
#endif

#else

#include <sys/time.h>

#endif
#endif
