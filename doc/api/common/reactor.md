# NAME

`measurement_kit/common/reactor.hpp`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```C++
#ifndef MEASUREMENT_KIT_COMMON_REACTOR_HPP
#define MEASUREMENT_KIT_COMMON_REACTOR_HPP

struct event_base;

namespace mk {

class Reactor {
  public:
    static SharedPtr<Reactor> make();

    static SharedPtr<Reactor> global();

    virtual ~Reactor();

    virtual void call_in_thread(Callback<> &&cb) = 0;

    virtual void call_soon(Callback<> &&cb) = 0;

    virtual void call_later(double time, Callback<> &&cb) = 0;

    virtual void pollin_once(
            socket_t sockfd, double timeout, Callback<Error> &&cb) = 0;

    virtual void pollout_once(
            socket_t sockfd, double timeout, Callback<Error> &&cb) = 0;

    virtual event_base *get_event_base() = 0;

    void run_with_initial_event(Callback<> &&cb);

    virtual void run() = 0;

    virtual void stop() = 0;
};

} // namespace mk
#endif
```

# DESCRIPTION

`Reactor` reacts to I/O events and manages delayed calls. Most MK objects reference a specific Reactor. 

Reactor is an abstract interface because there may be different implementations. The default implementation uses libevent as backend. 

_Note_: Albeit Reactor allows to perform asynchronous I/O on sockets, by calling select() or equivalent, more performant system APIs, typically you want to use code in mk::net to implement asynchronous I/O. In fact, code in mk::net uses the proactor pattern that is more efficient to perform asynchronous I/O, especially under Windows. The feature exposed by Reactor is there mainly to interface with third-party libraries such as, for example, c-ares. 

Throughout the documentation we will call `I/O thread` the thread that is currently blocked in Reactor::run() dispatching events. 

Available since measurement-kit v0.1.0. 

Originally Reactor was called `Poller` but was renamed in MK v0.2.0. It was significantly reworked in MK v0.4.0, v0.7.0. and v0.8.0.

`make()` returns an instance of the default Reactor. _Note_: The first time a reactor is created, libevent is configured to be thread safe _and_, on Unix, we ignore SIGPIPE.

`global()` returns the global instance of the default Reactor.

`~Reactor()` destroys any allocated resources.

`call_in_thread()` schedules the execution of cb inside a background thread created on demand. A maximum of three such threads can be active at any time. Additionally scheduled callback will wait for a thread to be ready to serve them. When there are no further callbacks to execute, background threads will exit, to save resources. 

Throws std::exception (or a derived class) if it is not possible to create a background thread or schedule the callback. 

If cb throws an exception of type std::exception (or derived from it), such exception is swallowed.

`call_soon() schedules the execution of cb in the I/O thread as soon as possible. 

Throws std::exception (or a derived class) if it is not possible to schedule the callback. 

_BUG_: Any exception thrown by the callback will not be swallowed and will thus cause the stack to unwind.

`call_later()` is like `call_soon()` except that the callback is scheduled `time` seconds in the future. 

_BUG_: if time is negative, the callback will never be called.

`pollin_once()` will monitor sockfd for readability. Parameter sockfd is the socket to monitor for readability. On Unix system, this can actually be any file descriptor. Parameter timeout is the timeout in seconds. Passing a negative value will imply no timeout. Parameter cb is the callback to be called. The Error argument will be TimeoutError if the timeout expired, NoError otherwise.

`pollout_once()` is like pollin_once() but for writability.

`get_event_base()` returns libevent's event base. Throws std::exception (or a derived class) if the backend is not libevent and you are trying to access the event base. _Note_: we configure the event base to be thread safe using libevent API.

`run_with_initial_event` is syntactic sugar for calling call_soon() immediately followed by run().

`run()` blocks processing I/O events and delayed calls. Throws std::exception (or a derived class) if it is not possible to start the reactor. A common case where this happens is when the reactor is already running. _Note_: This function will return if there is no pending I/O and no delayed calls (either registered to run in background threads or in the I/O thread). This behavior changed in MK v0.8.0 before which run() blocked until stop() was called.

`stop()` signals to the I/O loop to stop. If the reactor is not running yet, this method has no effect. Throws std::exception (or a derived class) if it is not possible to stop the reactor.

