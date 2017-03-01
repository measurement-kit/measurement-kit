// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../net/emitter.hpp"

namespace mk {
namespace net {

EmitterBase::~EmitterBase() {
    if (close_cb) {
        close_cb();
    }
}

void EmitterBase::close(Callback<> cb) {
    if (close_pending) {
        return;
    }
    shutdown();
    close_pending = true;  // Must be after shutdown()
    on_connect(nullptr);
    on_data(nullptr);
    on_flush(nullptr);
    on_error(nullptr);
    close_cb = cb;
}

Emitter::~Emitter() {}

} // namespace net
} // namespace mk
