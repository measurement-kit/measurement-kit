// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef JNI_FUNCS_HPP
#define JNI_FUNCS_HPP

#include <jni.h>

#include <string>

jboolean call_boolean_method(JNIEnv *env, jobject obj, const char *name);

jlong call_long_method(JNIEnv *env, jobject obj, const char *name);

void call_void_method(JNIEnv *env, jobject obj, const char *name);

void delete_global_ref(JNIEnv *env, jobject obj);

jfieldID get_field_id(JNIEnv *env, jobject obj, const char *name,
                      const char *type);

jlong get_long_attr(JNIEnv *env, jobject obj, const char *name);

jmethodID get_method_id(JNIEnv *env, jobject obj, const char *name,
                        const char *type);

JNIEnv *get_nonlocal_jnienv();

std::string get_string_attr(JNIEnv *env, jobject obj, const char *name);

std::string make_cxxstring(JNIEnv *env, jstring jstr);

jstring make_jstring(JNIEnv *env, const char *str);

jobject new_global_ref(JNIEnv *env, jobject obj);

#endif // JNI_FUNCS_HPP
