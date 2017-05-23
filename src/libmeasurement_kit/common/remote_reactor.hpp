// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_COMMON_REMOTE_REACTOR_HPP
#define SRC_LIBMEASUREMENT_KIT_COMMON_REMOTE_REACTOR_HPP

#include <measurement_kit/common.hpp>

#include <future>

namespace mk {

class RemoteReactor : public Reactor {
  public:
    RemoteReactor() {
        reactor_ = Reactor::make();
        worker_ = Worker::make();
        worker_->set_parallelism(1);
    }

    // Note: here we don't stop the I/O loop because we want to give the
    // caller the guarantee that he/she can dispose of this object once
    // done with it without stopping all the background I/O (of couse if
    // the current object is lost it might be difficult to continue to
    // use the background thread, but this is life :-).
    ~RemoteReactor() override {}

    void call_soon(Callback<> cb) override {
        // Guarantee: since we move all the arguments, this object can die
        // after this function returns without any hazard.
        with_running_reactor_and_locked_do_([cb = std::move(cb)](
            Var<Reactor> rr) { rr->call_soon(std::move(cb)); });
    }

    void call_later(double timeo, Callback<> cb) override {
        // Guarantee: since we move all the arguments, this object can die
        // after this function returns without any hazard.
        with_running_reactor_and_locked_do_(
            [ timeo = std::move(timeo), cb = std::move(cb) ](Var<Reactor> rr) {
                rr->call_later(std::move(timeo), std::move(cb));
            });
    }

    void loop_once() override {
        throw std::logic_error("loop_once() not available in this reactor");
    }

    void loop() override {
        throw std::logic_error("loop() not available in this reactor");
    }

    void loop_with_initial_event(Callback<>) override {
        throw std::logic_error(
            "loop_with_initial_event() not available in this reactor");
    }

    event_base *get_event_base() override {
        // XXX This in theory means that one can access the `event_base`
        // from another thread, however it should be thread safe...
        return locked(mutex_, [&]() { return reactor_->get_event_base(); });
    }

    void break_loop() override {
        // Here we don't need to ensure that the reactor is running.
        //
        // If, after this method, this object is still alive, the user can
        // call other methods that can eventually restart the thread.
        //
        // In the worst case, such operations will be scheduled for a little
        // while when the reactor is stopping (this will happen due to the
        // properties of the worker, which has a queue).
        locked(mutex_, [&]() {
            reactor_->break_loop();
            active_ = false;
        });
    }

    void pollfd(socket_t sockfd, short events, double timeout,
                Callback<Error, short> callback) override {
        // Guarantee: since we move all the arguments, this object can die
        // after this function returns without any hazard.
        with_running_reactor_and_locked_do_([
            sockfd = std::move(sockfd), events = std::move(events),
            callback = std::move(callback), timeout = std::move(timeout)
        ](Var<Reactor> rr) {
            rr->pollfd(std::move(sockfd), std::move(events),
                       std::move(callback), std::move(timeout));
        });
    }

  private:
    void with_running_reactor_and_locked_do_(Callback<Var<Reactor>> cb) {
        locked(mutex_, [ cb = std::move(cb), this ]() {
            // Guarantee: functions called with this method will be
            // called when we know that the reactor is running
            if (!active_) {
                std::promise<void> promise;
                std::future<void> future = promise.get_future();
                // Guarantee: the background thread only has access to the
                // thread safe reactor and keeps it alive via `Var<>`.
                //
                // Moreover: the worker is safe with respect to the worker
                // itself dying with the background thread running.
                worker_->run_in_background_thread([&promise, r = reactor_ ]() {
                    r->run_with_initial_event(
                        [&promise]() { promise.set_value(); });
                });
                future.wait();
                active_ = true;
            }
            // Note: passing the reactor as argument so we do not need to
            // capture `this` in the lambda capture above
            cb(reactor_);
        });
    }

    bool active_ = false;
    std::mutex mutex_;
    Var<Reactor> reactor_;
    Var<Worker> worker_;
};

} // namespace mk
#endif
