// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_REACTOR_HPP
#define MEASUREMENT_KIT_COMMON_REACTOR_HPP

#include <algorithm>                       // for std::move
#include <functional>                      // for std::function
#include <measurement_kit/common/safe.hpp> // for mk::Safe
#include <memory>                          // for std::shared_ptr

struct event_base;

namespace mk {

class Reactor {
  public:
    class Impl {
      public:
        virtual ~Impl();
        virtual void call_in_background_thread(std::function<void()> &&cb) = 0;
        virtual void call_later(double, std::function<void()> &&cb) = 0;
        virtual event_base *get_event_base() = 0;
        virtual void run() = 0;
        virtual void stop() = 0;
    };

    Reactor();

    void call_in_background_thread(std::function<void()> &&cb) const {
        impl_->call_in_background_thread(std::move(cb));
    }

    void call_soon(std::function<void()> &&cb) const {
        call_later(0.0, std::move(cb));
    }

    void call_later(double after, std::function<void()> &&cb) const {
        impl_->call_later(after, std::move(cb));
    }

    event_base *get_event_base() const { return impl_->get_event_base(); }

    void run_with_initial_event(std::function<void()> &&cb) const {
        call_soon(std::move(cb));
        run();
    }

    void run() const { impl_->run(); }

    void stop() const { impl_->stop(); }

    static Reactor global();

  private:
    Safe<std::shared_ptr<Impl>> impl_;
};

} // namespace mk
#endif
