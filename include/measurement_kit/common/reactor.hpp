// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_REACTOR_HPP
#define MEASUREMENT_KIT_COMMON_REACTOR_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/socket.hpp>
#include <measurement_kit/common/var.hpp>

// Deprecated since v0.4.x
struct event_base;

#define MK_POLLIN 1 << 0
#define MK_POLLOUT 1 << 1

namespace mk {

class Reactor {
  public:
    static Var<Reactor> make();
    static Var<Reactor> global();
    virtual ~Reactor();

    virtual void call_soon(Callback<> &&cb) = 0;
    virtual void call_later(double, Callback<> &&cb) = 0;

    // Backward compatibility names
    void loop_with_initial_event(Callback<> &&cb) {
        run_with_initial_event(std::move(cb));
    }
    void loop() { run(); }
    void break_loop() { stop(); }

    /*
        POSIX API for dealing with sockets. Slower than APIs in net,
        especially under Windows, but suitable to integrate with other
        async libraries such as c-ares and perhaps others.
    */
    virtual void pollfd(socket_t sockfd, short events, double timeout,
                        Callback<Error, short> &&callback) = 0;

    // Backward compatibility API
    void pollfd(socket_t sockfd, short events, Callback<Error, short> &&cb,
                double timeout = -1.0) {
        pollfd(sockfd, events, timeout, std::move(cb));
    }

    // Deprecated since v0.4.x
    virtual event_base *get_event_base() = 0;

    void run_with_initial_event(Callback<> &&cb);
    virtual void run() = 0;
    virtual void stop() = 0;
    virtual void set_autostop(bool) = 0;
    virtual bool autostop() = 0;

    /*
     * Note: run_task_*() adapted from nettests/runner.cpp@cd03860
     */

    template <typename Task, typename Callback>
    void run_task_deferred(const std::string &&task_name, Var<Logger> logger,
                           const Task &&task, const Callback &&cb) {
        logger->debug("task '%s': scheduled", task_name.c_str());
        /*
         * Step 1: defer execution of task. This ensures we run the task in
         * the I/O thread. We need to move everything such that the I/O thread
         * could obtain single ownership of all the variables (except logger
         * that can safely be shared among threads).
         */
        call_soon([
            task_name = std::move(task_name), logger, task = std::move(task),
            cb = std::move(cb), this
        ]() {
            run_task_now(std::move(task_name), logger, std::move(task),
                         std::move(cb));
        });
    }

    template <typename Task, typename Callback>
    void run_task_now(const std::string &&task_name, Var<Logger> logger,
                      const Task &&task, const Callback &&cb) {
        logger->debug("task '%s': start", task_name.c_str());
        /*
         * Step 2: call the task. Here we can safely copy because this
         * scope only contains the invocation of task. Just for robustness
         * copy everything explicitly rather than using `=`. We use the
         * lambda closure to keep the task alive.
         */
        task([task_name, logger, task, cb, this]() {
            logger->debug("task '%s': complete", task_name.c_str());
            /*
             * Step 3: schedule callback. We don't callback immediately
             * because the stack may contain invocations of methods of
             * task (or of objects owned by task). So, if the task dies
             * right after the real callback, we get a use after free. To
             * make sure memory is used correctly, schedule the task for
             * running as a call-soon lambda that keeps it alive. This
             * gives the guarantee that, below the point where such lambda
             * is called, there are only libevent functions on the stack.
             */
            call_soon([task_name, logger, task, cb]() {
                logger->debug("task '%s': callback", task_name.c_str());
                cb(std::move(task));
            });
        });
    }

    virtual bool is_running() = 0;
};

void call_soon(Callback<> &&, Var<Reactor> = Reactor::global());
void call_later(double, Callback<> &&, Var<Reactor> = Reactor::global());
void loop_with_initial_event(Callback<> &&, Var<Reactor> = Reactor::global());
void loop(Var<Reactor> = Reactor::global());
void break_loop(Var<Reactor> = Reactor::global());

// Introduced as aliases in v0.4.x
inline void run_with_initial_event(Callback<> &&callback,
                                   Var<Reactor> reactor = Reactor::global()) {
    loop_with_initial_event(std::move(callback), reactor);
}
inline void run(Var<Reactor> reactor = Reactor::global()) { loop(reactor); }
inline void stop(Var<Reactor> reactor = Reactor::global()) {
    break_loop(reactor);
}

} // namespace mk
#endif
