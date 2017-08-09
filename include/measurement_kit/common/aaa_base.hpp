// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_AAA_BASE_HPP
#define MEASUREMENT_KIT_COMMON_AAA_BASE_HPP

// Portability definition and headers

#include <measurement_kit/portable/sys/types.h>

#include <measurement_kit/portable/netinet/in.h>
#include <measurement_kit/portable/netinet/tcp.h>
#include <measurement_kit/portable/arpa/inet.h>
#include <measurement_kit/portable/sys/socket.h>
#include <measurement_kit/portable/sys/time.h>
#include <measurement_kit/portable/netdb.h>

#include <measurement_kit/portable/strings.h>

// Note: standard headers with non-standard additions
#include <measurement_kit/portable/stdlib.h>
#include <measurement_kit/portable/time.h>

// Include ciso646 to make sure we have `and`, `or`, etc
#include <ciso646>

#endif
