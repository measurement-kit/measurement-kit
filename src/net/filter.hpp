// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_FILTER_HPP
#define SRC_NET_FILTER_HPP

// Documentation: doc/private/net/filter.md

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/util.h>
#include <functional>
#include <limits.h>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>

struct evbuffer;
struct bufferevent;

extern "C" {

bufferevent_filter_result mk_filter_in(evbuffer *src, evbuffer *dst,
        ev_ssize_t count, bufferevent_flush_mode mode, void *);

bufferevent_filter_result mk_filter_out(evbuffer *src, evbuffer *dst,
        ev_ssize_t count, bufferevent_flush_mode mode, void *);

void mk_filter_free(void *);

} // extern "C"

namespace mk {
namespace net {

class Filter {
  public:
    Filter() {}
    ~Filter() {}

    typedef bufferevent_filter_result R;
    typedef bufferevent_flush_mode M;

    R emit_input(evbuffer *src, evbuffer *dst, ev_ssize_t count, M mode) {
        if (!filter_input_) {
            return pass_through_(src, dst, count, mode);
        }
        // Function wrappers always return void, hand roll safe code
        auto copy = filter_input_;
        return copy(src, dst, count, mode);
    }
    void on_input(std::function<R(evbuffer *, evbuffer *, ev_ssize_t, M)> cb) {
        filter_input_ = cb;
    }
    ErrorOr<int> trigger_input(M mode = BEV_FLUSH) {
        return trigger_(EV_READ, mode);
    }

    R emit_output(evbuffer *src, evbuffer *dst, ev_ssize_t count, M mode) {
        if (!filter_output_) {
            return pass_through_(src, dst, count, mode);
        }
        // Function wrappers always return void, hand roll safe code
        auto copy = filter_output_;
        return copy(src, dst, count, mode);
    }
    void on_output(std::function<R(evbuffer *, evbuffer *, ev_ssize_t, M)> cb) {
        filter_output_ = cb;
    }
    ErrorOr<int> trigger_output(M mode = BEV_FLUSH) {
        return trigger_(EV_WRITE, mode);
    }

    ErrorOr<int> trigger_(int what, M mode) {
        int rc = bufferevent_flush(bev, what, mode);
        if (rc == -1) {
            return BuffereventFlushError();
        }
        if (rc != 0 && rc != 1) {
            throw std::runtime_error("unexpected return value");
        }
        return rc;
    }

    R pass_through_(evbuffer *src, evbuffer *dst, ev_ssize_t count, M) {

        // If count is negative we read the maximum number we can read (i.e.
        // INT_MAX because of evbuffer_remove_buffer return type).
        // Likewise, if count is greater than INT_MAX, trim down count to
        // be the maximum number we can read (i.e. INT_MAX).
        int n = (count < 0 or count > INT_MAX) ? INT_MAX : (int)count;

        // By construction n is positive, so casting to unsigned is safe
        if (evbuffer_remove_buffer(src, dst, (unsigned)n) < 0) {
            return BEV_ERROR;
        }

        return BEV_OK;
    }

    bufferevent *bev = nullptr; // Weak ref to the object owning us

  private:
    std::function<R(evbuffer *, evbuffer *, ev_ssize_t, M)> filter_input_;
    std::function<R(evbuffer *, evbuffer *, ev_ssize_t, M)> filter_output_;
};

template <class Filter = Filter>
void filter_bufferevent(bufferevent *underlying,
        std::function<void(Error, Filter *)> cb) {
    Filter *filter = new Filter;
    if ((filter->bev = bufferevent_filter_new(underlying, mk_filter_in,
                 mk_filter_out, BEV_OPT_CLOSE_ON_FREE, mk_filter_free,
                 filter)) == nullptr) {
        bufferevent_free(underlying);
        delete filter;
        cb(BuffereventFilterNewError(), nullptr);
        return;
    }
    cb(NoError(), filter);
}

} // namespace net
} // namespace mk
#endif
