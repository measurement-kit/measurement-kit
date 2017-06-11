// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_LIBEVENT_POLLER_HPP
#define SRC_LIBMEASUREMENT_KIT_LIBEVENT_POLLER_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace libevent {

class Poller : public Reactor, public NonCopyable, public NonMovable {
  public:
    Poller();
    ~Poller() override;
    event_base *get_event_base() override;
    void call_soon(Callback<> &&) override;
    void call_later(double, Callback<> &&) override;
    void run() override;
    void loop_once();
    void stop() override;
    void pollfd(socket_t, short, double, Callback<Error, short> &&) override;

    // BEGIN internal functions used to test periodic event functionality
    void handle_periodic_();
    void on_periodic_(Callback<Poller *>);
    // END internal functions used to test periodic event functionality

    // Public because this simplifies unit testing and because this
    // class is not meant to be used directly but as Var<Reactor>
    Var<event_base> base_;
    Delegate<Poller *> periodic_cb_;
    bool autostop_ = false;
    bool is_running_ = false;
};

} // namespace libevent
} // namespace mk
#endif
