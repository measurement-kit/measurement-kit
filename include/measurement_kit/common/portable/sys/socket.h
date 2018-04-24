/*
 * Public domain.
 */
#ifndef MEASUREMENT_KIT_PORTABLE_SYS_SOCKET_H
#define MEASUREMENT_KIT_PORTABLE_SYS_SOCKET_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

#endif
