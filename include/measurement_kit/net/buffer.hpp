// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_NET_BUFFER_HPP
#define MEASUREMENT_KIT_NET_BUFFER_HPP

#include <measurement_kit/net/error.hpp>
#include <measurement_kit/common/evbuffer.hpp>

#include <event2/buffer.h>
#include <event2/util.h>

#include <functional>
#include <iosfwd>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>

#include <sys/types.h>
#include <sys/uio.h>
#include <stddef.h>
#include <string.h>

struct evbuffer;

namespace measurement_kit {
namespace net {

class Buffer {
  private:
    common::Evbuffer evbuf;

  public:
    Buffer(evbuffer *b = nullptr) {
        if (b != nullptr && evbuffer_add_buffer(evbuf, b) != 0)
            throw std::runtime_error("evbuffer_add_buffer failed");
    }

    ~Buffer() {}

    /*
     * I expect to read (write) from (into) the input (output)
     * evbuffer of a certain bufferevent. It seems to me natural
     * to use the insertion and extraction operators for that.
     */

    Buffer &operator<<(evbuffer *source) {
        if (source == nullptr) throw std::runtime_error("source is nullptr");
        if (evbuffer_add_buffer(evbuf, source) != 0)
            throw std::runtime_error("evbuffer_add_buffer failed");
        return *this;
    }

    Buffer &operator>>(evbuffer *dest) {
        if (dest == nullptr) throw std::runtime_error("dest is nullptr");
        if (evbuffer_add_buffer(dest, evbuf) != 0)
            throw std::runtime_error("evbuffer_add_buffer failed");
        return *this;
    }

    Buffer &operator<<(Buffer &source) {
        *this << source.evbuf;
        return *this;
    }

    Buffer &operator>>(Buffer &source) {
        *this >> source.evbuf;
        return *this;
    }

    size_t length() { return evbuffer_get_length(evbuf); }

    /*
     * The following is useful to feed a parser (e.g., the http-parser)
     * with all (or part of) the content of `this`.
     */
    void foreach(std::function<bool(const void *, size_t)> fn) {
        auto required_size = evbuffer_peek(evbuf, -1, nullptr, nullptr, 0);
        if (required_size < 0) throw std::runtime_error("unexpected error");
        if (required_size == 0) return;
        std::unique_ptr<evbuffer_iovec[]> raii;
        raii.reset(new evbuffer_iovec[required_size]); // Guarantee cleanup
        auto iov = raii.get();
        auto used = evbuffer_peek(evbuf, -1, nullptr, iov, required_size);
        if (used != required_size) throw std::runtime_error("unexpected error");
        for (auto i = 0; i < required_size &&
                fn(iov[i].iov_base, iov[i].iov_len); ++i) {
            /* nothing */ ;
        }
    }

    /*
     * Discard(), read(), readline() and readn() are the common operations
     * that you need to implement a protocol (AFAICT).
     */

    void discard(size_t count) {
        if (evbuffer_drain(evbuf, count) != 0)
            throw std::runtime_error("evbuffer_drain failed");
    }
    void discard() { discard(length()); }

    std::string readpeek(bool ispeek, size_t upto) {
        size_t nbytes = 0;
        std::string out;
        foreach([&nbytes, &out, &upto](const void *p, size_t n) {
            if (upto < n) n = upto;
            out.append((const char *) p, n);
            upto -= n;
            nbytes += n;
            return (upto > 0);
        });
        /*
         * We do this after foreach() because we are not supposed
         * to modify the underlying `evbuf` during foreach().
         */
        if (!ispeek) discard(nbytes);
        return out;
    }

    std::string read(size_t upto) { return readpeek(false, upto); }

    std::string read() { return read(length()); }

    std::string peek(size_t upto) { return readpeek(true, upto); }

    std::string peek() { return peek(length()); }

    /*
     * The semantic of readn() is that we return a string only
     * when we have exactly N bytes available.
     */
    std::string readn(size_t n) {
        if (n > length()) return "";
        return read(n);
    }

    std::tuple<common::Error, std::string> readline(size_t maxline) {

        size_t eol_length = 0;
        auto search_result =
            evbuffer_search_eol(evbuf, nullptr, &eol_length, EVBUFFER_EOL_CRLF);
        if (search_result.pos < 0) {
            if (length() > maxline)
                return std::make_tuple(EOLNotFoundError(), "");
            return std::make_tuple(0, "");
        }

        /*
         * Promotion to size_t safe because eol_length is a small
         * number and because we know that pos is non-negative.
         */
        if (eol_length != 1 && eol_length != 2)
            throw std::runtime_error("unexpected error");
        auto len = (size_t)search_result.pos + eol_length;
        if (len > maxline)
            return std::make_tuple(LineTooLongError(), "");

        return std::make_tuple(0, read(len));
    }

    /*
     * Wrappers for write, including a handy wrapper for sending
     * random bytes to the output stream.
     */

    void write(std::string in) { write(in.c_str(), in.length()); }

    Buffer &operator<<(std::string in) {
        write(in);
        return *this;
    }

    void write(const char *in) {
        if (in == nullptr) throw std::runtime_error("in is nullptr");
        write(in, strlen(in));
    }

    Buffer &operator<<(const char *in) {
        write(in);
        return *this;
    }

    void write(const void *buf, size_t count) {
        if (buf == nullptr) throw std::runtime_error("buf is nullptr");
        if (evbuffer_add(evbuf, buf, count) != 0)
            throw std::runtime_error("evbuffer_add failed");
    }

    void write_uint8(uint8_t);

    void write_uint16(uint16_t);

    void write_uint32(uint32_t);

    void write_rand(size_t count) {
        if (count == 0) return;
        char *p = new char[count];
        evutil_secure_rng_get_bytes(p, count);
        auto ctrl = evbuffer_add_reference(
            evbuf, p, count, [](const void *, size_t, void *p) {
                delete[] static_cast<char *>(p);
            }, p);
        if (ctrl != 0) throw std::runtime_error("evbuffer_add_reference");
    }

    void write(size_t count, std::function<size_t(void *, size_t)> func) {
        if (count == 0) return;
        char *p = new char[count];
        size_t used = func(p, count);
        if (used > count) {
            delete[] p;
            throw std::runtime_error("internal error");
        }
        if (used == 0) {
            delete[] p;
            return;
        }
        auto ctrl = evbuffer_add_reference(
            evbuf, p, used, [](const void *, size_t, void *p) {
                delete[] static_cast<char *>(p);
            }, p);
        if (ctrl != 0) throw std::runtime_error("evbuffer_add_reference");
    }

};

} // namespace net
} // namespace measurement_kit
#endif
