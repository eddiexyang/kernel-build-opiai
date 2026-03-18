KO_TYPE := HOST

ifeq ($(ARCH),arm64)
KO_ARCH_TYPE := arm64
else ifeq ($(ARCH), arm)
KO_ARCH_TYPE := arm
else
KO_ARCH_TYPE := x86
endif

include $(BUILD_SYSTEM)/kernel_module.mk

