// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/net/emitter.hpp"

namespace mk {
namespace net {

EmitterBase::~EmitterBase() {
    if (close_cb) {
        close_cb();
    }
}

void EmitterBase::close(Callback<> cb) {
    if (close_pending) {
        /*
         * Rationale for throwing rather than ignoring: (1) it was the
         * behavior before we started refactoring; (2) the user expects
         * a callback to be called after close is successful. If we do
         * ignore subsequent close attempts, the promise that the callback
         * will be called is silently broken, and it seems instead better
         * to inform the caller that there is a problem with the code
         * because `close()` has been called more than once.
         */
        throw std::runtime_error("close already pending");
    }
    close_pending = true;
    shutdown();
    on_connect(nullptr);
    on_data(nullptr);
    on_flush(nullptr);
    on_error(nullptr);
    close_cb = cb;
}

Emitter::~Emitter() {}

} // namespace net
} // namespace mk
