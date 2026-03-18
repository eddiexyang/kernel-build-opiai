ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef $(WDATE_TIME)
ccflags-y += -Wstack-usage=2048 -Wno-unused-parameter -Wno-sign-compare
ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif
EXTRA_CFLAGS += -DCONFIG_DEBUG_UDRV
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_UNIFORM_DIR)/src/include/kerndrv_api

ifneq ($(filter $(PRODUCT), ascend610Lite),)
    EXTRA_CFLAGS += -DASCEND_MDC_LITE
    obj-m += drv_i2c_uniform.o
    drv_i2c_uniform-objs := hisi_i2c.o i2c_v400.o i2c_misc.o i2c_misc_610.o
else
    obj-m += drv_i2c_uniform.o
    drv_i2c_uniform-objs := hisi_i2c.o i2c_v400.o i2c_misc.o i2c_misc_310.o
endif
