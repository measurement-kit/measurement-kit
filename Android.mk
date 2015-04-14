TOP_LOCAL_PATH := $(call my-dir)

include $(TOP_LOCAL_PATH)/src/ext/libevent/Android.mk

LOCAL_PATH := $(TOP_LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_STATIC_LIBRARIES := event2

LOCAL_C_INCLUDES := \
    src \
    src/ext/libevent/include \
    src/ext/libevent/android

LOCAL_CFLAGS := $(LOCAL_C_INCLUDES:%=-I%)

LOCAL_MODULE:= ight

LOCAL_CFLAGS := -DIGHT_ANDROID

LOCAL_SRC_FILES := \
    src/common/log.cpp \
    src/common/poller.cpp \
    src/common/stringvector.cpp \
    src/common/utils.cpp \
    src/ext/strtonum.c \
    src/net/connection.cpp \
    src/net/protocol.cpp

include $(BUILD_STATIC_LIBRARY)
