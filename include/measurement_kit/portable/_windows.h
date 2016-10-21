// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_PORTABLE__WINDOWS_H
#define MEASUREMENT_KIT_PORTABLE__WINDOWS_H

#include <ws2tcpip.h>
#include <winsock.h>

#ifdef _MSC_VER
# define __attribute__(x) /* nothing */
#endif

#endif
