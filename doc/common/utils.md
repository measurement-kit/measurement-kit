# NAME
Utils -- Utility functions.

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

if (!measurement_kit::socket_valid(sockfd)) {
    /* do something */
}

evutil_socket_t sockfd = measurement_kit::socket_normalize_if_invalid(fd);

timeval tv;
measurement_kit::timeval_now(&tv);
double now = measurement_kit::time_now();

evutil_socket_t so = measurement_kit::listen("PF_INET", "127.0.0.1", "80");

measurement_kit::xfree(ptr);

timeval tv, *tvp;
tvp = timeval_init(&tv, 17.144);

int ctrl;
sockaddr_storage storage;
socklen_t len;
memset(&storage, 0, sizeof (storage));
ctrl = measurement_kit::storage_init(&storage, &len, "PF_INET6",
                                     "::1", "8080");
ctrl = measurement_kit::storage_init(&storage, &len, PF_INET6,
                                     "::1", "8080");
ctrl = measurement_kit::storage_init(&storage, &len, PF_INET6,
                                     "::1", 8080);
if (ctrl != 0) {
    /* handle error */
}

evutil_socket_t sock = measurement_kit::socket_create(PF_INET, SOCK_STREAM, 0);

ctrl = measurement_kit::socket_connect(sockfd, &storage, len);
if (ctrl != 0) {
    /* handle error */
}

ctrl = measurement_kit::socket_listen(sockfd, &storage, len);
if (ctrl != 0) {
    /* handle error */
}

std::string measurement_kit::random_str(512);

std::string measurement_kit::random_str_uppercase(512);
```

# DESCRIPTION

The `utils` module contains low-level utility functions used throughout
MeasurementKit code.

# HISTORY

The `utils` module appeared in MeasurementKit 0.1.
