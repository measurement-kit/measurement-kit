// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "ooni.h"
#include "jni_funcs.hpp"

#include <jni.h>

#include <measurement_kit/ooni.hpp>

using namespace measurement_kit::ooni;

extern "C" {

JNIEXPORT jlong JNICALL
Java_io_github_measurement_1kit_ooni_TCPConnect_allocTest
  (JNIEnv *env, jobject obj) {
    try {
        auto file_path = get_string_attr(env, obj, "mFilePath");
        auto port = get_string_attr(env, obj, "mPort");
        return (jlong) new TCPConnect(file_path, {{"port", port}});
    } catch (...) {
        /* XXX suppress */
        return (jlong) 0L;
    }
}

JNIEXPORT jlong JNICALL
Java_io_github_measurement_1kit_ooni_DNSInjection_allocTest
  (JNIEnv *env, jobject obj) {
    try {
        auto file_path = get_string_attr(env, obj, "mFilePath");
        auto ns = get_string_attr(env, obj, "mNameServer");
        return (jlong) new DNSInjection(file_path, {{"nameserver", ns}});
    } catch (...) {
        /* XXX suppress */
        return (jlong) 0L;
    }
}

JNIEXPORT jlong JNICALL
Java_io_github_measurement_1kit_ooni_HTTPInvalidRequestLine_allocTest
  (JNIEnv *env, jobject obj) {
    try {
        auto backend = get_string_attr(env, obj, "mBackend");
        return (jlong) new HTTPInvalidRequestLine({{"backend", backend}});
    } catch (...) {
        /* XXX suppress */
        return (jlong) 0L;
    }
}

} // extern "C"
