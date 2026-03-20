ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char -Wformat=2 -Wstack-usage=2048 -Wcast-align -Wextra
ccflags-y += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-sign-compare

EXTRA_CFLAGS += -I$(TOP_DIR)/ace/dvpp/inc/dvpp
EXTRA_CFLAGS += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/base/ext_inc
EXTRA_CFLAGS += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/base/arch/hidvpp/include
EXTRA_CFLAGS += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/include
EXTRA_CFLAGS += -I$(TOP_DIR)/ace/dvpp/dvpp/HiDvpp/cbb/include/adapt
EXTRA_CFLAGS += -I$(TOP_DIR)/ace/media/dss/vdp/include

ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

obj-m += drv_media_compat.o
drv_media_compat-objs := media_compat.o
