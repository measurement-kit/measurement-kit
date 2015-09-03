// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_DELAYED_CALL_HPP
#define MEASUREMENT_KIT_COMMON_DELAYED_CALL_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/libs.hpp>
#include <measurement_kit/common/pointer.hpp>

#include <functional>

struct event;
struct event_base;

namespace measurement_kit {
namespace common {

// TODO: after MeasurementKit 0.1 move this class inside the .cpp file
class DelayedCallState : public NonCopyable, public NonMovable {
  public:
    DelayedCallState(double, std::function<void()>, Libs *libs = nullptr,
                     event_base *evbase = nullptr);
    ~DelayedCallState();

  private:
    /*
     * A previous implementation of this class required `func` to
     * be a pointer. The current implementation does not. So we can
     * rewrite the code to use an object rather than a pointer.
     */
    std::function<void()> *func_ = nullptr;
    event *evp_ = nullptr;
    Libs *libs_ = Libs::global();

    // Callback for libevent
    static void dispatch(evutil_socket_t, short, void *);
};

class DelayedCall {
  public:
    DelayedCall() {}
    DelayedCall(double delay, std::function<void()> func,
                Libs *libs = nullptr, event_base *evbase = nullptr) {
        state_.reset(new DelayedCallState(delay, func, libs, evbase));
    }
  private:
    SharedPointer<DelayedCallState> state_;
};

} // namespace common
} // namespace measurement_kit
#endif
