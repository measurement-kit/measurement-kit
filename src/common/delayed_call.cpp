// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/common/delayed_call.hpp"
#include <measurement_kit/common/libs.hpp>
#include <measurement_kit/common/poller.hpp>
#include "src/common/utils.hpp"

#include <event2/event.h>
#include <event2/util.h>

#include <new>
#include <stdexcept>

#include <sys/select.h> // for struct timeval

namespace mk {

DelayedCallState::DelayedCallState(double t, std::function<void()> f,
                                   Libs *libs, event_base *evbase) {
    timeval timeo;
    if (libs != nullptr) libs_ = libs;
    if (evbase == nullptr) evbase = get_global_event_base();
    if ((evp_ = libs_->event_new(evbase, MEASUREMENT_KIT_SOCKET_INVALID,
                                 EV_TIMEOUT, dispatch, this)) == nullptr) {
        throw std::bad_alloc();
    }
    if (libs_->event_add(evp_, timeval_init(&timeo, t)) != 0) {
        libs_->event_free(evp_);
        throw std::runtime_error("cannot register new event");
    }
    func_ = f;
}

void DelayedCallState::dispatch(evutil_socket_t, short, void *opaque) {
    auto state = static_cast<DelayedCallState *>(opaque);
    if (state->func_) state->func_();
}

DelayedCallState::~DelayedCallState() {
    if (evp_ == nullptr) return;
    libs_->event_free(evp_);
    evp_ = nullptr;
    func_ = nullptr;
}

} // namespace mk
