// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_REACTOR_HPP
#define MEASUREMENT_KIT_COMMON_REACTOR_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
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
    // ## Factory methods

    /// `make()` returns an instance of the default Reactor.
    /// \note The first time a reactor is created, libevent is configured
    /// to be thread safe _and_, on Unix, we ignore SIGPIPE.
    static SharedPtr<Reactor> make();

    /// `global()` returns the global instance of the default Reactor.
    static SharedPtr<Reactor> global();

    // ## Delayed calls

    /// `~Reactor()` destroys any allocated resources.
    virtual ~Reactor();

    /// \brief `call_in_thread()` schedules the execution of \p cb
    /// inside a background thread created on demand. A maximum
    /// of three such threads can be active at any time. Additionally
    /// scheduled callback will wait for a thread to be ready to
    /// serve them. When there are no further callbacks to execute,
    /// background threads will exit, to save resources.
    ///
    /// The \p logger parameter is the logger to be used.
    ///
    /// \throw std::exception (or a derived class) if it is not
    /// possible to create a background thread or schedule the callback.
    ///
    /// If \p cb throws an exception, this exception propagates.
    virtual void call_in_thread(SharedPtr<Logger> logger, Callback<> &&cb) = 0;

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

    // ## Poll sockets

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

    // ## Handles

    // A handle is a opaque references to transport abstractions. Internally
    // the reactor has unique ownership of a transport-like structure. Different
    // handle types have different internal structure but are all accessed via
    // their handle in an opaque way.
    //
    // A handle is a `uint64_t` number. The reactor will never issue two equal
    // handles and will throw an exception if it runs out of handles (given
    // the amount of handles in a `uint64_t`, this should not happen). This is
    // to prevent reuse of already closed handles. You can choose whether
    // accessing a non-openned handle will result on the operation being ignored
    // (this is the default) or in an exception being thrown. This setting does
    // not apply to the `close` operation, which is always idempotent.
    //
    // If you don't close handles, the reactor will automatically close them
    // when it exits. However, for efficiency, it is better to close the open
    // handles when you are done with them. Internally, the reactor keeps a
    // map mapping an open handle to its specific internal structure. Already
    // closed handles are not included in such map.
    //
    // Given that we except a reactor to be used for single, albeit long
    // operations like nettests and orchestration, and given that we do not
    // more than 1-10 sockets to be openned at a given time, maintaining a
    // map between the handle and the underlying structure has going to have
    // a negligible runtime cost (seeking into a very sparse map). Yet, if
    // you do not close openned handles, this seek operation may become more
    // expensive, especially for nettests with input like Web Connectivity.
    using handle_t = uint64_t;

    // `accessing_invalid_handles_is_fatal` allows you to control whether using
    // a non-openned handle is ignored (the default, and the behavior you get by
    // passing `false` to this method) or causes a std::runtime_exception to
    // be thrown (the behavior you get by passing `true` to this method). Note
    // that this setting does not apply to the `close` operation because closing
    // an handle that is not open is idempotent.
    void accessing_invalid_handles_is_fatal(bool enable) = 0;

    // `attach_tcp_socket` creates an handle suitable for managing a TCP
    // socket. On Unix, you can also pass other stream-oriented file descriptors
    // here like, for example, the standard input (i.e. filedesc 0). On Win32
    // it will only work with `SOCKET` handles. This family of handles will not
    // work reliably with UDP or other datagram sockets.
    handle_t attach_tcp_socket(socket_t sock) = 0;

    // `new_tcp_socket` creates an handle suitable for a TCP socket. All the
    // caveats listed in `attach_tcp_socket` documentation apply. Once the
    // socket has been created, you can connect it using `connect`.
    handle_t new_tcp_socket() = 0;

    // `connect` starts an asynchronous connect operation. This method is going
    // to fail if the handle family does not support connect, or if the socket
    // attached to the handle is already connected. Address must be an IPv4
    // or IPv6 address, and not a domain name. The callback will not be called
    // directly by this method; even in case of immediate error, it will be
    // scheduled to be called "soon". If connect succeeds the callback will be
    // called with NoError as argument, otherwise an error will be passed.
    void connect(handle_t handle, std::string &&address, uint16_t port,
            Callback<Error> &&cb) = 0;

    // `set_timeout` sets the timeout for I/O operations (in seconds as a double
    // precision floating point number). When an I/O operation like recv() or
    // send() or connect() takes more than the specified timeout, it will fail
    // with `TimeoutError` passed as error to the operation callback. By default
    // a timeout of ten seconds will be used for all I/O operations.
    void set_timeout(handle_t handle, double timeout) = 0;

    // `read` schedules a single read operation for the specified handle. The
    // callback argument will be called at a later time, when the read completes
    // or there is an I/O error (not necessarily on the reading path). This
    // method will never call the read callback: even in case a result is
    // immediately available, the callback will be scheduled to be called in the
    // next I/O loop cycle. Calling this method once will cause the reactor to
    // start polling the underlying socket for readability (typically using an
    // efficient method, e.g., `epoll` or `kqueue` or, on Windows, overlapped
    // I/O and IOCP -- in general, this will not be implemented by us but by
    // libevent or whatever underlying async I/O library we're using). Calling
    // this method again from its callback will instruct the reactor to keep
    // polling, while not calling this method again from its callback will cause
    // instead the reactor to stop polling for readability. Calling this method
    // in its callback with `nullptr` as argument, in particular, will cause the
    // current callback to be reused to handle the next read event. After an
    // error has occurred, you need to call read again if you want to start
    // reading again. Depending on the error and on the socket type, this may
    // or may not work.
    void read(handle_t handle, Callback<Error, const void *, size_t> &&cb) = 0;

    // `write` appends data to an internal queue associated to the handle. You
    // can call this method multiple times to append further data. Note that
    // this method will not write any data on the underlying socket. To do that,
    // you need to call the `flush` method, described below. Note that it is
    // okay to call this method regardless of whether a flush operation is in
    // progress or not. If the handle is attached to a datagram oriented socket,
    // each write will set a packet boundary. Otherwise, if the socket is a
    // stream socket, the code may not respect the boundary of each write.
    void write(handle_t handle, std::string &&data) = 0;

    // `flush` starts flushing the internal queue, sending data over the
    // underlying socket. You can append to the queue when a flush operation
    // is in progress. The callback will be called when the data queue has
    // become empty or when an error occurs. Calling flush from the flush
    // callback is legal an will cause a subsequent flush attempt to start
    // immediately. Calling flush again from outside its callback will
    // not interrupt the ongoing flush but will update the final callback
    // to be called when the flush operation will be complete. Note that
    // the callback may be called for I/O errors happening outside of the
    // write path, typically because it does not make sense to continue
    // writing as the connection has been lost. After an error has occurred,
    // you need to call flush again if you want to restart flush. This may
    // or may not work depending on the error that you received.
    void flush(handle_t, Callback<Error> &&cb) = 0;

    // `close` closes an open handle. Further attempts to access this handle
    // will be ignored. The close operation is idempotent and it will not fail
    // even if you have configured that `accessing_invalid_handles_is_fatal`.
    void close(handle_t handle) = 0;

    // ## Loop API

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
