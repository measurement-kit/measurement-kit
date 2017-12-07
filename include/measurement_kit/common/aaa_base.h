// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_AAA_BASE_H
#define MEASUREMENT_KIT_COMMON_AAA_BASE_H

/// This header file contains portability definitions and headers. You
/// should not include portability headers directly, rather include this
/// file in your portable C/C++ code.

#include <measurement_kit/common/portable/sys/types.h>

#include <measurement_kit/common/portable/netinet/in.h>
#include <measurement_kit/common/portable/netinet/tcp.h>
#include <measurement_kit/common/portable/arpa/inet.h>
#include <measurement_kit/common/portable/arpa/nameser.h>
#include <measurement_kit/common/portable/sys/socket.h>
#include <measurement_kit/common/portable/sys/time.h>
#include <measurement_kit/common/portable/netdb.h>

#include <measurement_kit/common/portable/strings.h>

// Note: standard headers with non-standard additions
#include <measurement_kit/common/portable/stdlib.h>
#include <measurement_kit/common/portable/time.h>

#endif
