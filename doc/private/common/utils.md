# NAME
Utils -- Utility functions.

# SYNOPSIS
```C++
#include "src/common/utils.hpp"

if (!mk::socket_valid(sockfd)) {
    /* do something */
}

evutil_socket_t sockfd = mk::socket_normalize_if_invalid(fd);

timeval tv;
mk::timeval_now(&tv);
double now = mk::time_now();

evutil_socket_t so = mk::listen("PF_INET", "127.0.0.1", "80");

mk::xfree(ptr);

timeval tv, *tvp;
tvp = timeval_init(&tv, 17.144);

int ctrl;
sockaddr_storage storage;
socklen_t len;
memset(&storage, 0, sizeof (storage));
ctrl = mk::storage_init(&storage, &len, "PF_INET6",
                                     "::1", "8080");
ctrl = mk::storage_init(&storage, &len, PF_INET6,
                                     "::1", "8080");
ctrl = mk::storage_init(&storage, &len, PF_INET6,
                                     "::1", 8080);
if (ctrl != 0) {
    /* handle error */
}

evutil_socket_t sock = mk::socket_create(PF_INET, SOCK_STREAM, 0);

ctrl = mk::socket_connect(sockfd, &storage, len);
if (ctrl != 0) {
    /* handle error */
}

ctrl = mk::socket_listen(sockfd, &storage, len);
if (ctrl != 0) {
    /* handle error */
}

std::string mk::random_str(512);

std::string mk::random_str_uppercase(512);
```

# DESCRIPTION

The `utils` module contains low-level utility functions used throughout
MeasurementKit code.

# HISTORY

The `utils` module appeared in MeasurementKit 0.1.0.
