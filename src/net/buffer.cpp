// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <event2/buffer.h>
#include <netinet/in.h>                        // for htonl, htons
#include <stdint.h>                            // for uint16_t, uint32_t, etc
#include <functional>                          // for function
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <memory>                              // for unique_ptr
#include <stdexcept>                           // for runtime_error
#include <string>                              // for string, basic_string
#include "src/net/evbuffer.hpp"

namespace mk {
namespace net {

Buffer::Buffer() {
    evbuf = make_shared_evbuffer();
}

Buffer::Buffer(evbuffer *b) : Buffer() {
    if (b != nullptr && evbuffer_add_buffer(evbuf.get(), b) != 0) {
        throw std::runtime_error("evbuffer_add_buffer failed");
    }
}

/*static*/ Var<Buffer> Buffer::make() { return Var<Buffer>(new Buffer); }

Buffer::Buffer(std::string s) : Buffer() {
    write(s);
}

Buffer::Buffer(const void *p, size_t n) : Buffer() {
    write(p, n);
}

Buffer &Buffer::operator<<(evbuffer *source) {
    if (source == nullptr) throw std::runtime_error("source is nullptr");
    if (evbuffer_add_buffer(evbuf.get(), source) != 0)
        throw std::runtime_error("evbuffer_add_buffer failed");
    return *this;
}

Buffer &Buffer::operator>>(evbuffer *dest) {
    if (dest == nullptr) throw std::runtime_error("dest is nullptr");
    if (evbuffer_add_buffer(dest, evbuf.get()) != 0)
        throw std::runtime_error("evbuffer_add_buffer failed");
    return *this;
}

Buffer &Buffer::operator<<(Buffer &source) {
    *this << source.evbuf.get();
    return *this;
}

Buffer &Buffer::operator>>(Buffer &dest) {
    *this >> dest.evbuf.get();
    return *this;
}

size_t Buffer::length() { return evbuffer_get_length(evbuf.get()); }

void Buffer::for_each(std::function<bool(const void *, size_t)> fn) {
    auto required = evbuffer_peek(evbuf.get(), -1, nullptr, nullptr, 0);
    if (required < 0) throw std::runtime_error("unexpected error");
    if (required == 0) return;
    std::unique_ptr<evbuffer_iovec[]> raii;
    raii.reset(new evbuffer_iovec[required]); // Guarantee cleanup
    auto iov = raii.get();
    auto used = evbuffer_peek(evbuf.get(), -1, nullptr, iov, required);
    if (used != required) throw std::runtime_error("unexpected error");
    for (auto i = 0; i < required && fn(iov[i].iov_base, iov[i].iov_len); ++i) {
        /* nothing */;
    }
}

void Buffer::discard(size_t count) {
    if (evbuffer_drain(evbuf.get(), count) != 0)
        throw std::runtime_error("evbuffer_drain failed");
}

std::string Buffer::readpeek(bool ispeek, size_t upto) {
    size_t nbytes = 0;
    std::string out;
    for_each([&nbytes, &out, &upto](const void *p, size_t n) {
        if (upto < n) n = upto;
        out.append((const char *)p, n);
        upto -= n;
        nbytes += n;
        return (upto > 0);
    });
    /*
     * We do this after for_each() because we are not supposed
     * to modify the underlying `evbuf` during for_each().
     */
    if (!ispeek) discard(nbytes);
    return out;
}

ErrorOr<std::string> Buffer::readline(size_t maxline) {

    size_t eol_length = 0;
    auto search_result =
        evbuffer_search_eol(evbuf.get(), nullptr, &eol_length, EVBUFFER_EOL_CRLF);
    if (search_result.pos < 0) {
        if (length() > maxline) {
            return EOLNotFoundError();
        }
        return std::string();
    }

    /*
     * Promotion to size_t safe because eol_length is a small
     * number and because we know that pos is non-negative.
     */
    if (eol_length != 1 && eol_length != 2) {
        throw std::runtime_error("unexpected error");
    }
    auto len = (size_t)search_result.pos + eol_length;
    if (len > maxline) {
        return LineTooLongError();
    }
    return read(len);
}

void Buffer::write(const void *buf, size_t count) {
    if (buf == nullptr) throw std::runtime_error("buf is nullptr");
    if (evbuffer_add(evbuf.get(), buf, count) != 0)
        throw std::runtime_error("evbuffer_add failed");
}

ErrorOr<uint8_t> Buffer::read_uint8() {
    uint8_t value = 0;
    if (length() < sizeof (value)) {
        return NotEnoughDataError();
    }
    std::string str = read(sizeof (value));
    memcpy(&value, str.data(), sizeof (value));
    return value;
}

void Buffer::write_uint8(uint8_t num) { write(&num, sizeof(num)); }

ErrorOr<uint16_t> Buffer::read_uint16() {
    uint16_t value = 0;
    if (length() < sizeof (value)) {
        return NotEnoughDataError();
    }
    std::string str = read(sizeof (value));
    memcpy(&value, str.data(), sizeof (value));
    value = ntohs(value);
    return value;
}

void Buffer::write_uint16(uint16_t num) {
    num = htons(num);
    write(&num, sizeof(num));
}

ErrorOr<uint32_t> Buffer::read_uint32() {
    uint32_t value = 0;
    if (length() < sizeof (value)) {
        return NotEnoughDataError();
    }
    std::string str = read(sizeof (value));
    memcpy(&value, str.data(), sizeof (value));
    value = ntohl(value);
    return value;
}

void Buffer::write_uint32(uint32_t num) {
    num = htonl(num);
    write(&num, sizeof(num));
}

void Buffer::write_rand(size_t count) {
    if (count == 0) return;
    char *p = new char[count];
    evutil_secure_rng_get_bytes(p, count);
    auto ctrl = evbuffer_add_reference(
        evbuf.get(), p, count, [](const void *, size_t, void *p) {
            delete[] static_cast<char *>(p);
        }, p);
    if (ctrl != 0) throw std::runtime_error("evbuffer_add_reference");
}

void Buffer::write(size_t count, std::function<size_t(void *, size_t)> func) {
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
    auto ctrl = evbuffer_add_reference(evbuf.get(), p,
                                       used, [](const void *, size_t, void *p) {
                                           delete[] static_cast<char *>(p);
                                       }, p);
    if (ctrl != 0) throw std::runtime_error("evbuffer_add_reference");
}

} // namespace net
} // namespace mk
