// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_DELAYED_CALL_HPP
#define MEASUREMENT_KIT_COMMON_DELAYED_CALL_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/libs.hpp>

#include <functional>

struct event;
struct event_base;

namespace measurement_kit {
namespace common {

class DelayedCall : public NonCopyable, public NonMovable {
  public:
    DelayedCall(double, std::function<void()> &&, Libs *libs = nullptr,
                event_base *evbase = nullptr);
    ~DelayedCall();

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

} // namespace common
} // namespace measurement_kit
#endif
