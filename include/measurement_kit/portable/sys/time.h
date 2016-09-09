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

#else

#include <sys/time.h>

#endif
#endif
