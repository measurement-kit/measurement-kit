// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_NET_BUFFER_HPP
#define MEASUREMENT_KIT_NET_BUFFER_HPP

#include <stddef.h>                             // for size_t
#include <stdint.h>                             // for uint16_t, uint32_t, etc
#include <string.h>                             // for strlen
#include <functional>                           // for function
#include <measurement_kit/common/evbuffer.hpp>  // for Evbuffer
#include <measurement_kit/common/maybe.hpp>     // for Maybe
#include <measurement_kit/common/var.hpp>       // for Evbuffer
#include <stdexcept>                            // for runtime_error
#include <string>                               // for string
#include <tuple>                                // for tuple
struct evbuffer;

namespace mk {

class Error;

namespace net {

class Buffer {
  private:
    Var<Evbuffer> evbuf{new Evbuffer};

  public:
    Buffer(evbuffer *b = nullptr);

    ~Buffer() {}

    /*
     * I expect to read (write) from (into) the input (output)
     * evbuffer of a certain bufferevent. It seems to me natural
     * to use the insertion and extraction operators for that.
     */

    Buffer &operator<<(evbuffer *source);

    Buffer &operator>>(evbuffer *dest);

    Buffer &operator<<(Buffer &source) {
        *this << *source.evbuf;
        return *this;
    }
    Buffer &operator>>(Buffer &source) {
        *this >> *source.evbuf;
        return *this;
    }

    size_t length();

    /*
     * The following is useful to feed a parser (e.g., the http-parser)
     * with all (or part of) the content of `this`.
     */
    void for_each(std::function<bool(const void *, size_t)> fn);

    /*
     * Discard(), read(), readline() and readn() are the common operations
     * that you need to implement a protocol (AFAICT).
     */

    void discard(size_t count);
    void discard() { discard(length()); }

    std::string readpeek(bool ispeek, size_t upto);

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

    Maybe<std::string> readline(size_t maxline);

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

    void write(const void *buf, size_t count);

    void write_uint8(uint8_t);

    void write_uint16(uint16_t);

    void write_uint32(uint32_t);

    void write_rand(size_t count);

    void write(size_t count, std::function<size_t(void *, size_t)> func);
};

} // namespace net
} // namespace mk
#endif
