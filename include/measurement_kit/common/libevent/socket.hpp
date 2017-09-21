// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_LIBEVENT_SOCKET_HPP
#define MEASUREMENT_KIT_COMMON_LIBEVENT_SOCKET_HPP

#include <cassert>
#include <deque>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/util.h>
#include <measurement_kit/common/detail/mock.hpp>
#include <measurement_kit/common/detail/utils.hpp>
#include <measurement_kit/common/reactor.hpp>
#include <measurement_kit/common/socket.hpp>
#include <mutex>

extern "C" {
static inline void mk_socket_delete_string(const void *, size_t, void *);
static inline void mk_socket_connect(bufferevent *, short, void *);
static inline void mk_socket_event(bufferevent *, short, void *);
static inline void mk_socket_flush(bufferevent *, void *);
static inline void mk_socket_read(bufferevent *, void *);
}

namespace mk {
inline namespace libevent {

template <MK_MOCK(evbuffer_new), MK_MOCK(evbuffer_add_reference),
        MK_MOCK(evbuffer_add_buffer), MK_MOCK(evbuffer_pullup),
        MK_MOCK(evbuffer_get_length)>
class LibeventSocketBuffer : public mk::SocketBuffer {
  public:
    LibeventSocketBuffer() {
        if ((evbuf_ = evbuffer_new()) == nullptr) {
            throw std::runtime_error("evbuffer_new");
        }
    }

    LibeventSocketBuffer(std::string &&data) : LibeventSocketBuffer{} {
        std::string *pstring = new std::string{std::move(data)};
        if (evbuffer_add_reference(evbuf_, pstring->data(), pstring->size(),
                    mk_socket_delete_string, pstring) != 0) {
            delete pstring;
            throw std::runtime_error("evbuffer_add_reference");
        }
    }

    LibeventSocketBuffer(evbuffer *data) : LibeventSocketBuffer{} {
        if (evbuffer_add_buffer(evbuf_, data) != 0) {
            throw std::runtime_error("evbuffer_add_buffer");
        }
    }

    evbuffer *as_evbuffer() override { return evbuf_; }

    size_t size() override { return evbuffer_get_length(evbuf_); }

    const void *data() override {
        const void *ptr = evbuffer_pullup(evbuf_, -1);
        if (ptr == nullptr && evbuffer_get_length(evbuf_) > 0) {
            throw std::runtime_error("evbuffer_pullup");
        }
        return ptr;
    }

    ~LibeventSocketBuffer() override { evbuffer_free(evbuf_); }

    evbuffer *evbuf_;
};

// WARNING: the fact that we're using `enable_shared_from_this` implies this
// class MUST be instantiated using `std::make_shared`.
template <MK_MOCK(bufferevent_socket_new), MK_MOCK(bufferevent_enable),
        MK_MOCK(bufferevent_disable), MK_MOCK(bufferevent_write_buffer)>
class LibeventSocket : public std::enable_shared_from_this<Socket>,
                       public Socket {
  public:
    void set_default_callbacks_() {
        bufferevent_setcb(
                bev_, mk_socket_read, mk_socket_flush, mk_socket_event, this);
    }

    static std::shared_ptr<LibeventSocket> make_internal_(socket_t sockfd,
            SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
        auto sock = std::make_shared<LibeventSocket>();
        sock->bev_ = bufferevent_socket_new(reactor->get_event_base(), sockfd,
                BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
        if (sock->bev_ == nullptr) {
            throw std::bad_alloc();
        }
        sock->reactor_ = reactor;
        sock->logger_ = logger;
        // Keepalive this object until we're ready to dispose of it.
        sock->self_ = SharedPtr<Socket>{sock->shared_from_this()};
        sock->set_default_callbacks_();
        logger->debug2("socket %lld: alloc", sock->fd());
        return sock;
    }

    static SharedPtr<Socket> attach(socket_t sockfd, SharedPtr<Reactor> reactor,
            SharedPtr<Logger> logger) {
        return make_internal_(sockfd, reactor, logger)->self_;
    }

    static void connect(sockaddr *sa, socklen_t salen, Settings settings,
            SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
            Callback<Error, SharedPtr<Socket>> &&callback) {
        auto sock = make_internal_(-1, reactor, logger);
        sock->connect_cb_ = std::move(callback);
        // Override the default callbacks with connect specific ones
        bufferevent_setcb(
                sock->bev_, nullptr, nullptr, mk_socket_connect, sock.get());
        auto failed = bufferevent_socket_connect(sock->bev_, sa, salen);
        ErrorOr<double> timeout = settings.get_noexcept("net/timeout", 30.0);
        Error err;
        if (failed) {
            logger->debug2("socket %lld: connect failed", sock->fd());
            err = GenericError("connect");
        } else if (!timeout) {
            err = timeout.as_error();
        } else {
            /* NOTHING */;
        }
        if (!!err) {
            sock->bufferevent_connect_(BEV_EVENT_ERROR, err);
            return;
        }
        sock->set_timeout(*timeout);
        logger->debug2("socket %lld: connect in progress", sock->fd());
    }

    void set_timeout(double timeout) override {
        if (!closed_) {
            timeval tv, *tvp = timeval_init(&tv, timeout);
            if (bufferevent_set_timeouts(bev_, tvp, tvp) != 0) {
                throw std::runtime_error("bufferevent_set_timeouts");
            }
            logger_->debug2("socket %lld: set timeout: %lf", fd(), timeout);
        }
    }

    long long fd() override {
        // FIX: always return the file descriptor even when we're closed
        // otherwise the logging is going to become very confusing.
        return (long long)bufferevent_getfd(bev_);
    }

    void read(Callback<Error, SharedPtr<SocketBuffer>> &&cb) override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        if (closed_) {
            reactor_->call_soon([cb = std::move(cb)]() {
                cb(ValueError("closed socket"), mk::SocketBuffer::make());
            });
            return;
        }
        if (rh_.empty()) {
            if (bufferevent_enable(bev_, EV_READ) != 0) {
                throw std::runtime_error("bufferevent_enable");
            }
            logger_->debug2("socket %lld: start reading", fd());
        }
        rh_.push_back(std::move(cb));
    }

    void read_callback_unlocked_(Error error) {
        if (rh_.empty()) {
            throw std::runtime_error("no readers");
        }
        rh_.front()(error, mk::SocketBuffer::make(bufferevent_get_input(bev_)));
        rh_.pop_front();
        if (!closed_ && rh_.empty()) {
            if (bufferevent_disable(bev_, EV_READ) != 0) {
                throw std::runtime_error("bufferevent_disable");
            }
            logger_->debug2("socket %lld: stop reading", fd());
        }
    }

    class PendingWrite {
      public:
        SharedPtr<SocketBuffer> buff;
        Callback<Error> cb;
        PendingWrite(SharedPtr<SocketBuffer> buff, Callback<Error> &&cb)
            : buff{buff}, cb{std::move(cb)} {}
    };

    void write(SharedPtr<SocketBuffer> buff, Callback<Error> &&cb) override {
        // Implementation note: since we keep a strict queue of writes as
        // requested by the upper layer, this implementation is also suitable
        // to hold a DATAGRAM socket because we respect packet boundaries.
        std::unique_lock<std::recursive_mutex> _{mutex_};
        if (closed_) {
            reactor_->call_soon([cb = std::move(cb)]() {
                cb(ValueError("closed socket"));
            });
            return;
        }
        if (buff->size() <= 0) {
            reactor_->call_soon([cb = std::move(cb)]() { cb(NoError()); });
            return;
        }
        wh_.push_back(PendingWrite{buff, std::move(cb)});
        if (wh_.size() == 1) {
            logger_->debug2("socket %lld: start writing", fd());
            do_write_unlocked_();
        }
    }

    void do_write_unlocked_() {
        auto evbuf = wh_.front().buff->as_evbuffer();
        // Note: writing into the bufferevent also registers EV_WRITE
        if (bufferevent_write_buffer(bev_, evbuf) != 0) {
            throw std::runtime_error("bufferevent_write_buffer");
        }
    }

    void write_callback_unlocked_(Error error) {
        if (wh_.empty()) {
            throw std::runtime_error("no writers");
        }
        wh_.front().cb(error);
        wh_.pop_front();
        if (!closed_ && !wh_.empty()) {
            do_write_unlocked_();
        } else if (wh_.empty()) {
            logger_->debug2("socket %lld: stop writing", fd());
        }
    }

    Error map_error_unlocked_(short what) {
        Error err;
        // FIXME: error reporting here is very basic and incomplete
        if ((what & BEV_EVENT_ERROR) != 0) {
            err = GenericError("error");
            // TODO: make sure we always map with a nonzero error
        } else if ((what & BEV_EVENT_TIMEOUT) != 0) {
            err = GenericError("timeout");
        } else if ((what & BEV_EVENT_EOF) != 0) {
            err = GenericError("eof");
        } else {
            /* NOTHING */;
        }
        return err;
    }

    void bufferevent_connect_(short what, Error err) {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        if (!connect_cb_) {
            throw std::runtime_error("no connect callback");
        }
        if (what != 0 && !err) {
            err = map_error_unlocked_(what);
        }
        logger_->debug2("socket %lld: connect callback", fd());
        Callback<Error, SharedPtr<Socket>> callback;
        std::swap(callback, connect_cb_);
        if (!!err) {
            // Pass upstream a socket that doesn't need explicit close(); we
            // don't need to lock because we've already locked.
            close_unlocked_();
        } else {
            // Be prepared for reading and writing
            set_default_callbacks_();
        }
        callback(err, self_);
    }

    void bufferevent_event_(short what) {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        Error err = map_error_unlocked_(what);
        if (!closed_ && (what & BEV_EVENT_READING) != 0) {
            logger_->debug2("socket %lld: readable", fd());
            read_callback_unlocked_(err);
        }
        // FIXME: should we pass EofError() to the write callback?
        if (!closed_ && (what & BEV_EVENT_WRITING) != 0) {
            logger_->debug2("socket %lld: flushed", fd());
            write_callback_unlocked_(err);
        }
    }

    void close_unlocked_() {
        if (closed_) {
            return;
        }
        // Close must be idempotent.
        closed_ = true;
        // Put bufferevent in a state in which it should neither be registered
        // for polling read and/or write nor upcall us.
        if (bufferevent_disable(bev_, EV_READ | EV_WRITE) != 0) {
            throw std::runtime_error("bufferevent_disable");
        }
        bufferevent_setcb(bev_, nullptr, nullptr, nullptr, nullptr);
        logger_->debug2("socket %lld: shutdown (read and write)", fd());
        // For robustness, defer destruction to a moment where there is no
        // bufferevent code below us on the call stack for sure. Destruction
        // may happen later than that, if there are more pieces of code who
        // keep alive the current socket by referencing it.
        //
        // Deferring destruction is also important because in many cases
        // close_unlocked_() is called from a context in which we hold the
        // mutex, which SHOULD NOT be held when we destruct the socket,
        // or libc++ will throw an exception.
        reactor_->call_soon([this]() { self_.reset(); });
    }

    void close() override {
        std::unique_lock<std::recursive_mutex> _{mutex_};
        close_unlocked_();
    }

    // Implementation note: no need to lock mutex because we are not touching
    // any internal structure inside of this method
    SharedPtr<Reactor> reactor() override { return reactor_; }

    // Implementation note: no need to lock mutex because we are not touching
    // any internal structure inside of this method
    SharedPtr<Logger> logger() override { return logger_; }

    ~LibeventSocket() override {
        logger_->debug2("socket %lld: free", fd());
        bufferevent_free(bev_);
    }

    bufferevent *bev_;
    bool closed_ = false;
    Callback<Error, SharedPtr<Socket>> connect_cb_;
    std::recursive_mutex mutex_;
    SharedPtr<Logger> logger_;
    SharedPtr<Reactor> reactor_;
    std::deque<Callback<Error, SharedPtr<SocketBuffer>>> rh_;
    SharedPtr<Socket> self_;
    std::deque<PendingWrite> wh_;
};

} // namespace libevent
} // namespace mk

static inline void mk_socket_delete_string(const void *, size_t, void *ptr) {
    std::string *pstring = static_cast<std::string *>(ptr);
    delete pstring;
}

static inline void mk_socket_connect(bufferevent *bev, short what, void *ptr) {
    auto sock = static_cast<mk::LibeventSocket<> *>(ptr);
    if (bev != sock->bev_) {
        throw std::runtime_error("bufferevent mismatch");
    }
    // No need to do this because we defer destruction using call_soon
    // such that it happens later in another stack context:
    //
    // auto self = sock->self_;
    sock->bufferevent_connect_(what, mk::NoError());
}

static inline void mk_socket_event(bufferevent *bev, short what, void *ptr) {
    auto sock = static_cast<mk::LibeventSocket<> *>(ptr);
    if (bev != sock->bev_) {
        throw std::runtime_error("bufferevent mismatch");
    }
    // No need to do this because we defer destruction using call_soon
    // such that it happens later in another stack context:
    //
    // auto self = sock->self_;
    sock->bufferevent_event_(what);
}

static inline void mk_socket_flush(bufferevent *bev, void *ptr) {
    mk_socket_event(bev, BEV_EVENT_WRITING, ptr);
}

static inline void mk_socket_read(bufferevent *bev, void *ptr) {
    mk_socket_event(bev, BEV_EVENT_READING, ptr);
}
#endif
