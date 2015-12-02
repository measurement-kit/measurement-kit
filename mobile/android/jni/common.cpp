// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "common.h"
#include "jni_funcs.hpp"

#include <measurement_kit/common.hpp>

#include <android/log.h>
#include <jni.h>

#include <set>

using namespace measurement_kit::common;

class Context {
  public:
    std::set<NetTest *> to_delete;
    std::set<jobject> to_unref;
};

static void save_log(JNIEnv *env, jobject test, const char *s) {
    auto method_id = get_method_id(env, test, "appendLogLine",
            "(Ljava/lang/String;)V");
    jstring str = make_jstring(env, s);
    env->CallVoidMethod(test, method_id, str);
    // TODO: check for Java exception
}

extern "C" {

JNIEXPORT void JNICALL Java_io_github_measurement_1kit_common_Async_runTest
  (JNIEnv *env, jobject async, jobject test, jobject testComplete) {
    measurement_kit::debug("Async::runTest()...");
    auto tp = (NetTest *)nullptr;
    try {
        // XXX Here we leak two global refs in case of error
        test = new_global_ref(env, test);
        testComplete = new_global_ref(env, testComplete);
        tp = (NetTest *)call_long_method(env, test, "allocTest");
        if (tp == nullptr) throw std::bad_alloc();
        jboolean isnoisy = call_boolean_method(env, test, "getVerbose");
        if (isnoisy) tp->set_verbose(1);
        auto ctx = (Context *)get_long_attr(env, async, "mPointer");

        tp->on_log([test](const char *s) {
            try {
                __android_log_print(ANDROID_LOG_INFO, "mk::test-logger",
                                    "%s", s);
                JNIEnv *env = get_nonlocal_jnienv();
                save_log(env, test, s);
            } catch (...) {
                /* XXX suppress */
            }
        });

        tp->begin([ctx, test, testComplete, tp]() {
            tp->end([ctx, test, testComplete, tp]() {
                try {
                    ctx->to_unref.insert(testComplete);
                    ctx->to_unref.insert(test);
                    ctx->to_delete.insert(tp);
                    JNIEnv *env = get_nonlocal_jnienv();
                    call_void_method(env, testComplete, "run");
                } catch (...) {
                    /* XXX suppress */
                }
            });
        });

    } catch (...) {
        measurement_kit::debug("Async::runTest()... exception");
        delete tp;
        /* XXX suppress */
    }
    measurement_kit::debug("Async::runTest()... done");
}

JNIEXPORT void JNICALL Java_io_github_measurement_1kit_common_Async_loopOnce
  (JNIEnv *env, jobject async) {
    try {
        measurement_kit::loop_once();
        auto ctx = (Context *)get_long_attr(env, async, "mPointer");

        for (auto testptr : ctx->to_delete) {
            try {
                delete testptr;
            } catch (...) {
                /* XXX suppress */
            }
        }
        ctx->to_delete.clear();

        for (auto ref : ctx->to_unref) {
            try {
                delete_global_ref(env, ref);
            } catch (...) {
                /* XXX suppress */
            }
        }
        ctx->to_unref.clear();

    } catch (...) {
        /* XXX suppress */
    }
}

JNIEXPORT jlong JNICALL Java_io_github_measurement_1kit_common_Async_alloc
  (JNIEnv *, jclass) {
    try {
        return (jlong) new Context;
    } catch (...) {
        return (jlong) 0L;
    }
}

JNIEXPORT jlong JNICALL Java_io_github_measurement_1kit_common_NetTest_allocTest
  (JNIEnv *, jobject) {
    return (jlong) 0L;  /* derived classes must override this */
}

JNIEXPORT void JNICALL Java_io_github_measurement_1kit_common_Logger_setVerbose
  (JNIEnv *, jclass, jint verbose) {
    try {
        measurement_kit::set_verbose(verbose);
    } catch (...) {
        // XXX suppress
    }
}

JNIEXPORT void JNICALL
Java_io_github_measurement_1kit_common_Logger_useAndroidLogger
  (JNIEnv *, jclass) {
    try {
        measurement_kit::on_log([](const char *s) {
            __android_log_print(ANDROID_LOG_INFO, "mk::default-logger",
                                "%s", s);
        });
    } catch (...) {
        // XXX suppress
    }
}

} // extern "C"
