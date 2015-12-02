// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "jni_funcs.hpp"

#include <string>

#include <jni.h>
#include <android/log.h>

#include <measurement_kit/common.hpp>
#include <event2/dns.h>

static JavaVM *SAVED_VM = nullptr;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
    __android_log_print(ANDROID_LOG_INFO, "jni", "JNI_OnLoad...");
    SAVED_VM = vm;

    // TODO:
    // - understand whether JNIEnv could be cached
    // - understand how to cache field and method IDs
    // - if we want to register methods here rather than using
    //   the naming convention:
    //     - get JNIEnv...
    //     - Get jclass with env->FindClass.
    //     - Register methods with env->RegisterNatives.

    // XXX not clear what is the best way to get the DNS server
    // address on Android. My research shows that one could:
    //
    // 1. call getRuntime.exec() that can hang the app
    // 2. use reflection to use an hidden class
    //
    // source: http://stackoverflow.com/a/11362271
    //
    // Plus, probably we need to set DNS server when we run
    // a new test, since setting it when the app starts is not
    // optimal, as the device could change network
    //
    // Also, note that here there should be no DNS servers, but
    // for robustness we still remove nameservers just in case
    // there was a resolv.conf and then re-add them
    __android_log_print(ANDROID_LOG_INFO, "jni", "DNS server hack...");
    auto dns_base = measurement_kit::get_global_evdns_base();
    if (dns_base == nullptr ||
        evdns_base_clear_nameservers_and_suspend(dns_base) != 0 ||
        evdns_base_nameserver_ip_add(dns_base, "8.8.8.8") != 0 ||
        evdns_base_nameserver_ip_add(dns_base, "8.8.4.4") != 0 ||
        evdns_base_resume(dns_base) != 0) {
        throw std::runtime_error("cannot setup the DNS server");
    }
    __android_log_print(ANDROID_LOG_INFO, "jni", "DNS server hack... ok");

    __android_log_print(ANDROID_LOG_INFO, "jni", "JNI_OnLoad... ok");
    return JNI_VERSION_1_6;
}

jboolean call_boolean_method(JNIEnv *env, jobject obj, const char *name) {
    auto method_id = get_method_id(env, obj, name, "()Z");
    auto res = env->CallBooleanMethod(obj, method_id);
    // TODO: check for Java exception
    return res;
}

jlong call_long_method(JNIEnv *env, jobject obj, const char *name) {
    auto method_id = get_method_id(env, obj, name, "()J");
    auto res = env->CallLongMethod(obj, method_id);
    // TODO: check for Java exception
    return res;
}

void call_void_method(JNIEnv *env, jobject obj, const char *name) {
    auto method_id = get_method_id(env, obj, name, "()V");
    env->CallVoidMethod(obj, method_id);
    // TODO: check for Java exception
}

void delete_global_ref(JNIEnv *env, jobject obj) {
    env->DeleteGlobalRef(obj);
}

jfieldID get_field_id(JNIEnv *env, jobject obj, const char *name,
                      const char *type) {
    // TODO: memoize?
    auto clazz = env->GetObjectClass(obj);
    if (!clazz) throw std::runtime_error("cannot get object class");
    auto fid = env->GetFieldID(clazz, name, type);
    if (!fid) throw std::runtime_error("cannot get field id");
    return fid;
}

jlong get_long_attr(JNIEnv *env, jobject obj, const char *name) {
    auto field_id = get_field_id(env, obj, name, "J");
    auto res = env->GetLongField(obj, field_id);
    // Note: assuming that long fields always store pointers
    if (res == 0L) throw std::runtime_error("invalid long field");
    return res;
}

jmethodID get_method_id(JNIEnv *env, jobject obj, const char *name,
                      const char *type) {
    // TODO: memoize?
    auto clazz = env->GetObjectClass(obj);
    if (!clazz) throw std::runtime_error("cannot get object class");
    auto mid = env->GetMethodID(clazz, name, type);
    if (!mid) throw std::runtime_error("cannot get method id");
    return mid;
}

JNIEnv *get_nonlocal_jnienv() {
    JNIEnv *env = nullptr;
    if (SAVED_VM == nullptr) throw std::runtime_error("invalid JVM");
    if (SAVED_VM->GetEnv(reinterpret_cast<void**>(&env),
                         JNI_VERSION_1_6) != JNI_OK) {
        throw std::runtime_error("cannot get nonlocal JNIEnv");
    }
    return env;
}

std::string get_string_attr(JNIEnv *env, jobject obj, const char *name) {
    auto field_id = get_field_id(env, obj, name, "Ljava/lang/String;");
    auto res = env->GetObjectField(obj, field_id);
    if (res == nullptr) throw std::runtime_error("invalid object field");
    return make_cxxstring(env, (jstring)res);
}

std::string make_cxxstring(JNIEnv *env, jstring jstr) {
    const char *s = env->GetStringUTFChars(jstr, nullptr);
    if (!s) throw std::bad_alloc();
    std::string copy = s;
    env->ReleaseStringUTFChars(jstr, s);
    return copy;
}

jstring make_jstring(JNIEnv *env, const char *s) {
    auto ret = env->NewStringUTF(s);
    if (!ret) throw std::bad_alloc();
    return ret;
}

jobject new_global_ref(JNIEnv *env, jobject obj) {
    auto res = env->NewGlobalRef(obj);
    if (!res) throw std::bad_alloc();
    return res;
}
