// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_AAA_BASE_HPP
#define MEASUREMENT_KIT_COMMON_AAA_BASE_HPP

// Portability definition and headers

#include <measurement_kit/portable/private/macros.h>
#include <measurement_kit/portable/sys/types.h>

#include <measurement_kit/portable/netinet/in.h>
#include <measurement_kit/portable/arpa/inet.h>
#include <measurement_kit/portable/sys/socket.h>
#include <measurement_kit/portable/sys/time.h>

#include <measurement_kit/portable/strings.h>
#include <measurement_kit/portable/unistd.h>

// Note: this is a standard header but still there are bits of it that
// are implemented differently under Windows (i.e. gmtime_r)
#include <measurement_kit/portable/time.h>

// Include ciso646 to make sure we have `and`, `or`, etc
#include <ciso646>

#endif
