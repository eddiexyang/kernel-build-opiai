ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef $(WDATE_TIME)
ccflags-y += -Wstack-usage=2048 -Wno-unused-parameter -Wno-sign-compare
ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif
EXTRA_CFLAGS += -DCONFIG_SPI_UDRV
EXTRA_CFLAGS += -I$(DRIVER_KERNEL_UNIFORM_DIR)/src/include/kerndrv_api

ifneq ($(filter $(PRODUCT), ascend610Lite),)
    EXTRA_CFLAGS += -DASCEND_MDC_LITE
    obj-m += drv_spi_uniform.o
    drv_spi_uniform-objs := hispi.o spi_hisiv500.o spi_api.o spi_misc.o spi_misc_610.o
else
    obj-m += drv_spi_uniform.o
    drv_spi_uniform-objs := hispi.o spi_hisiv500.o spi_api.o spi_misc.o spi_misc_310.o
endif
