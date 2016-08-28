LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libGeoIP
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/lib/libGeoIP.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcrypto
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/lib/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libevent
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/lib/libevent.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libevent_openssl
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/lib/libevent_openssl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libevent_pthreads
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/lib/libevent_pthreads.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libssl
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/lib/libssl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog -latomic
# Note to self: the order of libraries matters
LOCAL_STATIC_LIBRARIES := GeoIP ssl crypto event event_openssl event_pthreads
LOCAL_MODULE := measurement_kit
LOCAL_SRC_FILES := # filled by the include below
include include.mk
LOCAL_SRC_FILES += ../../../src/libmeasurement_kit/ext/http-parser/http_parser.c
LOCAL_SRC_FILES += ../../../src/libmeasurement_kit/ext/strtonum.c
LOCAL_SRC_FILES += ../../../src/libmeasurement_kit/ext/tls_verify.c
LOCAL_CPPFLAGS += -I $(TARGET_ARCH_ABI)/include -std=c++11 -I../../../include
LOCAL_CFLAGS += -I $(TARGET_ARCH_ABI)/include -I../../../include
include $(BUILD_SHARED_LIBRARY)
