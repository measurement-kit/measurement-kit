// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/delayed_call.hpp>
#include <measurement_kit/common/libs.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/utils.hpp>

#include <event2/event.h>
#include <event2/util.h>

#include <new>
#include <stdexcept>

#include <sys/select.h>  // for struct timeval

namespace measurement_kit {
namespace common {

DelayedCall::DelayedCall(double t, std::function<void()> &&f,
                         Libs *libs, event_base *evbase) {
    timeval timeo;

    if (libs != nullptr) libs_ = libs;
    if (evbase == nullptr) evbase = get_global_event_base();

    func_ = new std::function<void()>();

    if ((evp_ = libs_->event_new(evbase, MEASUREMENT_KIT_SOCKET_INVALID, EV_TIMEOUT,
                                       dispatch, func_)) == nullptr) {
        delete func_;
        throw std::bad_alloc();
    }

    if (libs_->event_add(evp_, timeval_init(&timeo, t)) != 0) {
        delete func_;
        libs_->event_free(evp_);
        throw std::runtime_error("cannot register new event");
    }

    std::swap(*func_, f);
}

void DelayedCall::dispatch(evutil_socket_t, short, void *opaque) {
    auto funcptr = static_cast<std::function<void()> *>(opaque);
    if (*funcptr) (*funcptr)();
}

DelayedCall::~DelayedCall(void) {
    delete (func_); /* delete handles nullptr */
    if (evp_) libs_->event_free(evp_);
}

} // namespace common
} // namespace measurement_kit
