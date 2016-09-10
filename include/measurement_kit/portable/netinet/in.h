/*
 * Public domain
 */
#ifndef MEASUREMENT_KIT_PORTABLE_NETINET_IN_H
#define MEASUREMENT_KIT_PORTABLE_NETINET_IN_H

#ifdef _WIN32

/*
 * 1. Pull `struct in_addr`.
 */
 #include <winsock2.h>

/*
 * 1. Pull `struct in6_addr`.
 */

 #include<ws2tcpip.h>

#else

#include <netinet/in.h>

#endif
#endif
