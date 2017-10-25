// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/version.h>
#include <openssl/opensslv.h>
#include <event2/event.h>

const char *mk_version() {
    return MK_VERSION;
}

const char *mk_version_full() {
    return MK_VERSION_FULL;
}

uint64_t mk_version_major(void) {
    return MK_VERSION_MAJOR;
}

uint64_t mk_version_minor(void) {
    return MK_VERSION_MINOR;
}

uint64_t mk_version_patch(void) {
    return MK_VERSION_PATCH;
}

uint64_t mk_version_stable(void) {
    return MK_VERSION_STABLE;
}

uint64_t mk_version_numeric(void) {
    return MK_VERSION_NUMERIC;
}

const char *mk_version_openssl() {
    return OPENSSL_VERSION_TEXT;
}

const char *mk_openssl_version() {
    return mk_version_openssl();
}

const char *mk_version_libevent() {
    return LIBEVENT_VERSION;
}

const char *mk_libevent_version() {
    return mk_version_libevent();
}
