// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <ares.h>

#include "../dns/ares_map_failure.hpp"

namespace mk {
namespace dns {

Error ares_map_failure(int status) {
    /*
     * Implementation note: for now we only map the error codes returned
     * by the `ares_expand_name()` function, the only one we use.
     */
    Error err = GenericError();
    switch (status) {
    case ARES_SUCCESS:
        err = NoError();
        break;
    case ARES_EBADNAME:
        err = BadNameError();
        break;
    case ARES_ENOMEM:
        err = OutOfMemoryError();
        break;
    default:
        /* NOTHING */
        break;
    }
    return err;
}

} // namespace dns
} // namespace mk
