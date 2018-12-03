// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/net/error.hpp"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <errno.h>
#endif

#include <assert.h>

namespace mk {
namespace net {

bool net_error_to_ooni_error(int code, std::string *str) noexcept {
    assert(str != nullptr);
#ifdef _WIN32
#define MAPPING(sys_name_, ooni_name_)                                         \
    case WSAE##sys_name_:                                                      \
        *str = #ooni_name_;                                                    \
        return true;
#else
#define MAPPING(sys_name_, ooni_name_)                                         \
    case E##sys_name_:                                                         \
        *str = #ooni_name_;                                                    \
        return true;
#endif
    switch (code) {
        MK_NET_ERRNO(MAPPING)
#ifndef _WIN32
        MK_NET_ERRNO_UNIX_ONLY(MAPPING)
#endif
    case 0:
        *str = "";
        return true;
    }
#undef MAPPING
    return false;
}

} // namespace net
} // namespace mk
