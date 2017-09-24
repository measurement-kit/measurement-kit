// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_REACTOR_HPP
#define MEASUREMENT_KIT_COMMON_REACTOR_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/shared_ptr.hpp>
#include <measurement_kit/common/socket.hpp>

struct event_base;

namespace mk {

/// \brief `Reactor` reacts to I/O events and manages delayed calls. Most MK
/// objects reference a specific Reactor.
///
/// Reactor is an abstract interface because there may be different
/// implementations. The default implementation uses libevent as backend.
///
/// \note Albeit Reactor allows to perform asynchronous I/O on sockets,
/// by calling select() or equivalent, more performant system APIs, typically
/// you want to use code in mk::net to implement asynchronous I/O. In fact,
/// code in mk::net uses the proactor pattern that is more efficient to
/// perform asynchronous I/O, especially under Windows. The feature exposed
/// by Reactor is there mainly to interface with third-party libraries
/// such as, for example, c-ares.
///
/// Throughout the documentation we will call `I/O thread` the thread that
/// is currently blocked in Reactor::run() dispatching events.
///
/// \since v0.1.0.
///
/// Originally Reactor was called `Poller` but was renamed in MK v0.2.0. It
/// was significantly reworked in MK v0.4.0, v0.7.0. and v0.8.0.
class Reactor {
  public:
    /// `make()` returns an instance of the default Reactor.
    /// \note The first time a reactor is created, libevent is configured
    /// to be thread safe _and_, on Unix, we ignore SIGPIPE.
    static SharedPtr<Reactor> make();

    /// `global()` returns the global instance of the default Reactor.
    static SharedPtr<Reactor> global();

    /// `~Reactor()` destroys any allocated resources.
    virtual ~Reactor();

    /// \brief `call_in_thread()` schedules the execution of \p cb
    /// inside a background thread created on demand. A maximum
    /// of three such threads can be active at any time. Additionally
    /// scheduled callback will wait for a thread to be ready to
    /// serve them. When there are no further callbacks to execute,
    /// background threads will exit, to save resources.
    ///
    /// \throw std::exception (or a derived class) if it is not
    /// possible to create a background thread or schedule the callback.
    ///
    /// If \p cb throws an exception of type std::exception (or
    /// derived from it), such exception is swallowed.
    virtual void call_in_thread(Callback<> &&cb) = 0;

    /// \brief `call_soon() schedules the execution of \p cb in the
    /// I/O thread as soon as possible.
    ///
    /// \throw std::exception (or a derived class) if it is not
    /// possible to schedule the callback.
    ///
    /// \bug Any exception thrown by the callback will not be swallowed
    /// and will thus cause the stack to unwind.
    virtual void call_soon(Callback<> &&cb) = 0;

    /// \brief `call_later()` is like `call_soon()` except that the callback
    /// is scheduled `time` seconds in the future.
    ///
    /// \bug if \p time is negative, the callback will never be called.
    virtual void call_later(double time, Callback<> &&cb) = 0;

    // Design note: I prefer separate pollin_once() and pollout_once()
    // operations to a single function call (previously it was called pollfd())
    // with flag, because the former approach allows you to have the equivalent
    // of two "threads" that can act independently, while with the latter
    // this seems to me to be more complicated to express. I guess this
    // may also be more a matter of taste than anything else.
    //
    // A secondary reason why it _may_ be better to have `pollin_once()` and
    // `pollout_once` may be that boost/asio can probably be included into the
    // standard C++ library. If that happens, having more basic methods
    // that seem more similar to boost/asio may help if we decide to use
    // the standard library implementation instead of libevent (but the
    // more I use it the more I find libevent good).

    /// \brief `pollin_once()` will monitor \p sockfd for readability.
    /// \note `pollin_once()` will poll the socket for readability
    /// just once. To poll multiple times, you must call this function
    /// multiple times (as the function name implies).
    /// \param sockfd is the socket to monitor for readability. On Unix
    /// system, this can actually be any file descriptor.
    /// \param timeout is the timeout in seconds. Passing a negative
    /// value will imply no timeout.
    /// \param cb is the callback to be called. The Error argument will
    /// be TimeoutError if the timeout expired, NoError otherwise.
    virtual void pollin_once(
            socket_t sockfd, double timeout, Callback<Error> &&cb) = 0;

    /// `pollout_once()` is like pollin_once() but for writability.
    virtual void pollout_once(
            socket_t sockfd, double timeout, Callback<Error> &&cb) = 0;

    /// \brief `get_event_base()` returns libevent's event base.
    /// \throw std::exception (or a derived class) if the backend is not
    /// libevent and you are trying to access the event base.
    /// \note we configure the event base to be thread safe using
    /// libevent API.
    virtual event_base *get_event_base() = 0;

    /// \brief `run_with_initial_event` is syntactic sugar for calling
    /// call_soon() immediately followed by run().
    void run_with_initial_event(Callback<> &&cb);

    /// \brief `run()` blocks processing I/O events and delayed calls.
    /// \throw std::exception (or a derived class) if it is not possible
    /// to start the reactor. A common case where this happens is when
    /// the reactor is already running.
    /// \note This function will return if there is no pending I/O and no
    /// delayed calls (either registered to run in background threads
    /// or in the I/O thread). This behavior changed in MK v0.8.0 before
    /// which run() blocked until stop() was called.
    virtual void run() = 0;

    /// \brief `stop()` signals to the I/O loop to stop. If the reactor
    /// is not running yet, this method has no effect.
    /// \throw std::exception (or a derived class) if it is not possible
    /// to stop the reactor.
    virtual void stop() = 0;
};

} // namespace mk
#endif
