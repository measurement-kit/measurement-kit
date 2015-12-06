# NAME
Buffer - Buffer containing data.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS

```C++
#include <measurement_kit/net.hpp>

// Constructor

mk::net::Buffer buf;

evbuffer *evbuf = /* something */ ;
mk::net::Buffer buf(evbuf);  // Move content from (evbuffer *) evbuf

// Move data from/to other buffers (this does not entail copies)

buf << bufferevent_get_input(bev);  // From bev to buf
buf >> bufferevent_get_output(bev);  // The other way round

Buffer another;
buf << another;  // Move data from another to buf
buf >> another;  // The other way round

// Basic building blocks for high level functions

size_t n = buf.length();

// Walk over data and decide when to stop by returning false
buf.for_each([](const char *p, size_t n) -> bool {
    // Do something with `p` and `n`
    return true;
});

// Discard, read and peek the buffer

buf.discard(128);
buf.discard();

std::string s = buf.read(128);
std::string s = buf.read();
std::string s = buf.peek(128);
std::string s = buf.peek();

std::string s = buf.readn(1024);
if (s == "") {
    // less than 1024 bytes buffered
    return;
}

mk::Maybe<std::string> res = buf.readline(1024);
if (!res) {
    // readline failed, check error
    auto error = res.as_error();
    return;
}
std::string s = res.as_value();

// Alternative way of reading lines (the "let it crash" way)
try {
    auto s = buf.readline(1024).as_value();
} catch (Error &error) {
    // TODO: process the error
}

// Write stuff

std::string s;
buf.write(s);
buff << s;

const char *s = "HTTP/1.1 200 Ok\r\n";
buf.write(s);
buf << s << "\r\n";

buf.write("ciao", 4);

buf.write_uint8('c');
buf.write_uint16(1284);
buf.write_uint32(7);

buf.write_rand(2048);

// Write directly into a 1024 chunk buffer of the output buffer chain
buf.write(1024, [](char *buf, size_t cnt) -> size_t {
    /* Write into `buf` up to `cnt` chars */
    return cnt;
});
```

# DESCRIPTION

The `Buffer` object encapsulates an `evbuffer`. It is used to efficiently
store data. You can move data from a `Buffer` to another `Buffer` at virtually
not cost (the same is true when moving data from/to an `evbuffer` since
that is internally used to implement a `Buffer`).

# HISTORY

The `Buffer` class appeared in MeasurementKit 0.1.0.
