// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_COMMON_POLLER_HPP
#define SRC_COMMON_POLLER_HPP

#include <functional>
#include <measurement_kit/common.hpp>

struct event_base;

namespace mk {

class Poller : public NonCopyable, public NonMovable, public Reactor {
  public:
    Poller(Libs *libs = nullptr);
    ~Poller();

    event_base *get_event_base() override { return base_; }

    /// Call the function at the beginning of next I/O loop.
    /// \param cb The function to be called soon.
    /// \throw Error if the underlying libevent call fails.
    void call_soon(std::function<void()> cb) override;

    void call_later(double, std::function<void()> cb) override;

    void loop_with_initial_event(std::function<void()> cb) override {
        call_soon(cb);
        loop();
    }

    void loop() override;

    void loop_once() override;

    void break_loop() override;

    // BEGIN internal functions used to test periodic event functionality
    void handle_periodic_();
    void on_periodic_(std::function<void(Poller *)> callback);
    // END internal functions used to test periodic event functionality

  private:
    event_base *base_;
    Libs *libs_ = get_global_libs();
    SafelyOverridableFunc<void(Poller *)> periodic_cb_;
};

} // namespace mk
#endif
