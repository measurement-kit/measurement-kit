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
    /*
     * Note: we share a `Var<State>` between the I/O thread and other
     * threads. According to the manual:
     *
     *     All member functions (including copy constructor and copy
     *     assignment) can be called by multiple threads on different
     *     instances of shared_ptr without additional synchronization
     *     even if these instances are copies and share ownership of
     *     the same object. If multiple threads of execution access
     *     the same shared_ptr without synchronization and any of those
     *     accesses uses a non-const member function of shared_ptr
     *     then a data race will occur; the shared_ptr overloads of
     *     atomic functions can be used to prevent the data race.
     *
     * Basically, we are safe as long as we don't change explicitly
     * the value that `state_` does point to.
     */
    class State {
      public:
        bool active = false;
        std::mutex mutex;
        Var<Reactor> reactor = Reactor::make();
        Var<Worker> worker = Worker::make();
    };

    RemoteReactor() {
        // Use parallelism 1 because we want to use just one thread
        state_->worker->set_parallelism(1);
        // Automatically stop, so we don't need to stop manually
        state_->reactor->set_autostop(true);
    }

    ~RemoteReactor() override {}

    void call_soon(Callback<> &&cb) override {
        // Guarantee: since we move all the arguments, this object can die
        // after this function returns without any hazard.
        with_running_reactor_and_locked_do_([cb = std::move(cb)](
            Var<Reactor> rr) { rr->call_soon(std::move(cb)); });
    }

    void call_later(double timeo, Callback<> &&cb) override {
        // Guarantee: since we move all the arguments, this object can die
        // after this function returns without any hazard.
        with_running_reactor_and_locked_do_([ timeo, cb = std::move(cb) ](
            Var<Reactor> rr) { rr->call_later(timeo, std::move(cb)); });
    }

    void run() override {
        throw std::runtime_error("run() not available in this reactor");
    }

    event_base *get_event_base() override {
        // Note: the underlying `event_base` is configured to be thread safe
        return locked(state_->mutex,
                      [&]() { return state_->reactor->get_event_base(); });
    }

    void stop() override {
        // Here we don't need to ensure that the reactor is running.
        //
        // If, after this method, this object is still alive, the user can
        // call other methods that can eventually restart the thread.
        //
        // In the worst case, such operations will be scheduled for a little
        // while when the reactor is stopping (this will happen due to the
        // properties of the worker, which has a queue).
        locked(state_->mutex, [&]() { reactor_->stop(); });
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

    void set_autostop(bool /*value*/) override {
        throw std::runtime_error("Cannot set auto-stop flag");
    }

    bool get_autostop() override { return true; }

    bool is_running() override {
        return locked(state_->mutex,
                      [&]() { return state_->reactor->is_running(); });
    }

  private:
    void with_running_reactor_and_locked_do_(Callback<Var<Reactor>> cb) {
        locked(mutex_, [ cb = std::move(cb), this ]() {
            // Guarantee: functions called with this method will be
            // called when we know that the reactor is running
            if (!state_->active) {
                std::promise<void> promise;
                std::future<void> future = promise.get_future();
                // Guarantee: the background thread only has access to the
                // state and keeps it alive via `Var<>`. This should be thread
                // safe behavior (see big comment on that above).
                //
                // Moreover: the worker is safe with respect to the worker
                // itself dying with the background thread running.
                worker_->run_in_background_thread([&promise, s = state_ ]() {
                    s->reactor->run_with_initial_event(
                        [&promise]() { promise.set_value(); });
                    locked(s->mutex, [&]() { s->active = false; });
                });
                future.wait();
                state_->active = true;
            }
            // Note: passing the reactor as argument so we do not need to
            // capture `this` in the lambda capture above
            cb(reactor_);
        });
    }

    Var<State> state_ = Var<State>::make();
};

} // namespace mk
#endif
