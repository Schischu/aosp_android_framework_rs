LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := renderscriptjni

LOCAL_SRC_FILES := renderscriptjni.cpp simple.rs

LOCAL_C_INCLUDES += $(JNI_H_INCLUDE)
LOCAL_C_INCLUDES += frameworks/rs/cpp
LOCAL_C_INCLUDES += frameworks/rs

LOCAL_CFLAGS := --std=c++11

LOCAL_CPP_FEATURES += exceptions

LOCAL_SHARED_LIBRARIES := libdl liblog libc++
LOCAL_STATIC_LIBRARIES := libRScpp_static

LOCAL_NDK_STL_VARIANT := gnustl_static

include $(BUILD_SHARED_LIBRARY)

