// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/net/filter.hpp"

using namespace mk::net;

extern "C" {

bufferevent_filter_result mk_filter_in(evbuffer *s, evbuffer *d, ev_ssize_t c,
        bufferevent_flush_mode m, void *o) {
    return static_cast<Filter *>(o)->emit_input(s, d, c, m);
}
bufferevent_filter_result mk_filter_out(evbuffer *s, evbuffer *d, ev_ssize_t c,
        bufferevent_flush_mode m, void *o) {
    return static_cast<Filter *>(o)->emit_output(s, d, c, m);
}
void mk_filter_free(void *o) { delete static_cast<Filter *>(o); }

} // extern "C"
