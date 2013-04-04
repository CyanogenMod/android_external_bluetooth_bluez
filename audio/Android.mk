LOCAL_PATH:= $(call my-dir)

# A2DP plugin

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	device.c \
	control.c \
	gateway.c \
	headset.c \
	media.c \
	ipc.c \
	main.c \
	manager.c \
	module-bluetooth-sink.c \
	sink.c \
	source.c \
	telephony-dummy.c \
	transport.c \

ifneq ($(BOARD_HAVE_BLUETOOTH_STE),true)
LOCAL_SRC_FILES+= \
	a2dp.c \
	avdtp.c \
	unix.c

else
LOCAL_SRC_FILES+= \
	ste-a2dp.c \
	ste-avdtp.c \
	ste-unix.c \
	ste-qos.c
endif

LOCAL_CFLAGS:= \
	-DVERSION=\"4.93\" \
	-DSTORAGEDIR=\"/data/misc/bluetoothd\" \
	-DCONFIGDIR=\"/etc/bluetooth\" \
	-DANDROID \
	-DNEED_G_SLIST_FREE_FULL \
	-Wno-missing-field-initializers \
	-Wno-pointer-arith \
	-DDEBUG_SECTION=\"_audio\" \
	-D__S_IFREG=0100000 # missing from bionic stat.h

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../gdbus \
	$(LOCAL_PATH)/../src \
	$(LOCAL_PATH)/../btio \
	$(call include-path-for, glib) \
	$(call include-path-for, dbus)

LOCAL_SHARED_LIBRARIES := \
	libbluetooth \
	libbluetoothd \
	libbtio \
	libdbus \
	libutils \
	libglib


LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/bluez-plugin
LOCAL_UNSTRIPPED_PATH := $(TARGET_OUT_SHARED_LIBRARIES_UNSTRIPPED)/bluez-plugin
LOCAL_MODULE := audio

include $(BUILD_SHARED_LIBRARY)

ifneq ($(BOARD_HAVE_BLUETOOTH_STE),true)

#
# liba2dp
# This is linked to Audioflinger so **LGPL only**

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	android_audio_hw.c \
	liba2dp.c \
	ipc.c \
	../sbc/sbc_primitives.c \
	../sbc/sbc_primitives_neon.c

ifeq ($(TARGET_ARCH),x86)
LOCAL_SRC_FILES+= \
	../sbc/sbc_primitives_mmx.c \
	../sbc/sbc.c
else
LOCAL_SRC_FILES+= \
	../sbc/sbc.c.arm \
	../sbc/sbc_primitives_armv6.c
endif

# to improve SBC performance
LOCAL_CFLAGS:= -funroll-loops

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../sbc \
	../../../../frameworks/base/include \
	system/bluetooth/bluez-clean-headers

LOCAL_SHARED_LIBRARIES := \
	libcutils

ifneq ($(wildcard system/bluetooth/legacy.mk),)
LOCAL_STATIC_LIBRARIES := \
	libpower

LOCAL_MODULE := liba2dp
else
LOCAL_SHARED_LIBRARIES += \
	libpower

LOCAL_MODULE := audio.a2dp.default
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
endif

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

else

#
# ALSA plugins
#

# These should be LGPL code only

#
# bluez plugin to ALSA PCM
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= ste-pcm_bluetooth.c ipc.c

LOCAL_CFLAGS:= -DPIC -D_POSIX_SOURCE -Wno-pointer-arith

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../alsa-lib/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../bluez/lib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sbc

LOCAL_SHARED_LIBRARIES := libcutils libutils libasound
LOCAL_STATIC_LIBRARIES := libsbc

# don't prelink this library
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/lib/alsa-lib
LOCAL_UNSTRIPPED_PATH := $(TARGET_OUT_UNSTRIPPED)/usr/lib/alsa-lib
LOCAL_MODULE := libasound_module_pcm_bluetooth
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)


#
# bluez plugin to ALSA CTL
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= ctl_bluetooth.c ipc.c

LOCAL_CFLAGS:= -DPIC -D_POSIX_SOURCE -Wno-pointer-arith

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../alsa-lib/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sbc

LOCAL_SHARED_LIBRARIES := libcutils libasound

LOCAL_LDFLAGS := -module -avoid-version

# don't prelink this library
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/lib/alsa-lib
LOCAL_UNSTRIPPED_PATH := $(TARGET_OUT_UNSTRIPPED)/usr/lib/alsa-lib
LOCAL_MODULE := libasound_module_ctl_bluetooth
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

endif
