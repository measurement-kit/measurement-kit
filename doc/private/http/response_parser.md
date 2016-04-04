# NAME
ResponseParser -- http response parser

# SYNOPSIS
```C++
#include "src/http/response_parser.hpp"

auto parser = measurement_kit::http::ResponseParser();

parser.on_begin([](void) {
    std::clog << "Begin of the response" << std::endl;
});

parser.on_headers_complete([](unsigned short major, unsigned short minor,
        unsigned int code, std::string& reason, std::map<std::string,
        std::string>&& headers) {
    std::clog << "HTTP/" << major << "." << minor
              << code << " " << reason << std::endl;
    for (auto pair : headers) {
        std::clog << pair.first << ": " << pair.second;
    }
    std::clog << "=== BEGIN BODY ===" << std::endl;
});

parser.on_body([](std::string&& part) {
    std::clog << "body part: " << part << std::endl;
});

parser.on_end([](void) {
    std::clog << "=== END BODY ===" << std::endl;
}

...

auto connection = measurement_kit::net::connect(...);

connection.on_data([&](SharedPointer<Buffer> data) {
    parser.feed(data);
});
```

# DESCRIPTION

This class implements an event-style HTTP response parser. The following
events are raised: `begin` when the response begins; `headers_complete`
when the HTTP headers were received; `body` when a piece of the body was
received; `end` when the response body was full received. Multiple
responses could be handled by the same parser.

# HISTORY

The `ResponseParser` class appeared in MeasurementKit 0.1.0.
