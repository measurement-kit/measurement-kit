# NAME
Buffer &mdash; Buffer containing data.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS

```C++
#include <measurement_kit/net.hpp>

namespace mk {
namespace net {

class Buffer {
  public:
    Buffer();
    Buffer(evbuffer *b);
    Buffer(std::string);
    Buffer(const void *, size_t);

    Buffer &operator<<(evbuffer *source);
    Buffer &operator>>(evbuffer *dest);
    Buffer &operator<<(Buffer &source);
    Buffer &operator>>(Buffer &source);

    size_t length();

    void for_each(std::function<bool(const void *, size_t)> fn);

    void discard(size_t count);
    void discard();
    std::string read(size_t upto);
    std::string read();
    std::string peek(size_t upto);
    std::string peek();

    std::string readn(size_t n);

    ErrorOr<std::string> readline(size_t maxline);

    void write(std::string in);
    Buffer &operator<<(std::string in);
    void write(const char *in);
    Buffer &operator<<(const char *in);
    void write(const void *buf, size_t count);

    ErrorOr<uint8_t> read_uint8();
    void write_uint8(uint8_t);
    ErrorOr<uint16_t> read_uint16();
    void write_uint16(uint16_t);
    ErrorOr<uint32_t> read_uint32();
    void write_uint32(uint32_t);

    void write_rand(size_t count);
    void write(size_t count, std::function<size_t(void *, size_t)> func);
} // namespace net
} // namespace mk
```

# DESCRIPTION

The `Buffer` type contains a list of strings either read from a socket or
to be written into a socket. It MAY be implemented using libevent's `evbuffer`.

The constructors allow to initialize, respectively, an empty `Buffer`, a
buffer from an existing libevent's `evbuffer` (this operation MAY be zero
copy if the `Buffer` is implemented using libevent's `evbuffer`), a C++
string, and a C style buffer (i.e. `void *` and `size_t`).

There are insertion and extraction operators from/to respectively a libevent's
`evbuffer` and a Buffer. Insertion and extraction to/from `Buffer` is always
zero copy, while insertion and extraction to/from libevent's `evbuffer` MAY be
zero copy if the `Buffer` is implemented using libevent's `evbuffer`.

The `length()` method returns the number of bytes in the `Buffer`.

The `for_each()` function allows to call a function on every string contained
by the `Buffer` object. To stop iterating earlier, the called function returns
`false`. Otherwise all the buffered strings are visited. This is useful to
inspect the content of the `Buffer` without triggering copies; e.g.:

```C++
    size_t total = 0;
    buffer.for_each([&](const void *ptr, size_t size) {
        process_data((char *)ptr, size);
        total += size;
        return (total <= 1024); /* Do not process more than 1,024 bytes */
    });
    buffer.discard(total);
```

Note that you MUST NOT modify the `Buffer` while iterating over it. The example
above shows the optimal pattern to discard data from the buffer after you have
finished iterating over it.

The `discard()`, `read()`, and `peek()` families of functions read from the
`Buffer`. As the name implies, the `discard()` functions discard data from
the `Buffer`; if the amount of bytes to discard is not specified, the whole
content of the buffer is discared. The `read()` functions extract data from the
buffer and returns is serialized as string; if the amount of bytes to read is
not specified, the whole content of the buffer would be extracted. The `peek()`
functions are like `read()` expect that the buffer content would not be
discared; as such, they are optimal to inspect (portions of) the buffer content
to decide whether special actions should be carried out. For example:

```C++
    std::string data = buffer.peek(4);
    if (data.size() < 4) {
        return -2; /* I.e. try again */
    }
    if (data == "HELO") {
        std::string helo = buffer.read(HELO_MESSAGE_LENGTH);
        if (helo.size() < HELO_MESSAGE_LENGTH) {
            return -2; /* I.e. try again */
        }
        /* Now process HELO's message content... */
        return 0;
    }
    /* Other cases... */
```

The `readn()` function returns either a N bytes string (where N is the number
of bytes passed as argument) or the empty string. This is useful to parse
protocol messages having a fixed width. For example, part of the previous example
could be rewritten as:

```C++
    if (data == "HELO") {
        std::string helo = buffer.readn(HELO_MESSAGE_LENGTH);
        if (helo == "") {
            return -2; /* I.e. try again */
        }
        /* Now process HELO's message content... */
        return 0;
    }
```

The `readline()` function reads a line no longer than the number of bytes
passed as its first argument. It returns the read line on success and an
error in case of failure.

The `Buffer` class contains write and insert operators that allow to queue
respectively a C++ string, a C string, and a C-style buffer.

The `Buffer` class also contains functionalities to read and write integers
of typical sizes (16 and 32 bits). In such case integers are automatically
converted from the host to network representation when writing and from network
to host representation when reading. Because read operations could fail, they
return `ErrorOr<T>` rather than just `T`.

The `write_rand()` method writes the specified number of random bytes
into the buffer.

The `write(size, callback)` allocates a buffer of size `size` and calls the
callback specified as second argument to fill it. Such callback MAY return
less than `size` bytes to indicate that less than `size` bytes have been
initialized by it. This is useful, for example, to allocate a buffer large
as the maximum message size but allowing, at the same time, to send smaller
messages.

# HISTORY

The `Buffer` class appeared in MeasurementKit 0.1.0.
