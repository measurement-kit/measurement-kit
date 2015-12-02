LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libevent
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libevent.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libyaml-cpp
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libyaml-cpp.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libmeasurement_kit
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libmeasurement_kit.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog -latomic
LOCAL_STATIC_LIBRARIES := measurement_kit event yaml-cpp
LOCAL_MODULE := measurement-kit-jni-0.0
LOCAL_SRC_FILES := \
	common.cpp \
	jni_funcs.cpp \
	ooni.cpp
APP_PLATFORM := android-21
LOCAL_CPPFLAGS += -I jni/$(TARGET_ARCH_ABI)/include -std=c++11
include $(BUILD_SHARED_LIBRARY)
